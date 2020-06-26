#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <string.h>
#include <assert.h>

#include "memory_fences.h"
#include "exttypes.h"
#include "spinlock.h"
#include "log.h"
#include "xalloc.h"

#include "hashtable/hashtable.h"
#include "hashtable/hashtable_op_set.h"
#include "hashtable/hashtable_support_hash.h"
#include "hashtable/hashtable_support_op.h"

bool hashtable_op_set(
        hashtable_t *hashtable,
        hashtable_key_data_t *key,
        hashtable_key_size_t key_size,
        hashtable_value_data_t value) {
    bool created_new;
    hashtable_hash_t hash;
    hashtable_chunk_index_t chunk_index = 0, chunk_index_start = 0;
    hashtable_half_hashes_chunk_volatile_t* half_hashes_chunk = 0;
    hashtable_key_value_volatile_t* key_value = 0;

    hashtable_key_data_t* ht_key;

    hash = hashtable_support_hash_calculate(key, key_size);

    LOG_DI("key (%d) = %s", key_size, key);
    LOG_DI("hash = 0x%016x", hash);

    // TODO: the resize logic has to be reviewed, the underlying hash search function has to be aware that it hasn't
    //       to create a new item if it's missing
    bool ret = hashtable_support_op_search_key_or_create_new(
            hashtable->ht_current,
            key,
            key_size,
            hash,
            true,
            &created_new,
            &chunk_index_start,
            &chunk_index,
            &half_hashes_chunk,
            &key_value);

    LOG_DI("created_new = %s", created_new ? "YES" : "NO");
    LOG_DI("half_hashes_chunk = 0x%016x", half_hashes_chunk);
    LOG_DI("key_value =  0x%016x", key_value);

    if (ret == false) {
        LOG_DI("key not found or not created, continuing");
        return false;
    }

    assert(key_value < hashtable->ht_current->keys_values + hashtable->ht_current->keys_values_size);

    LOG_DI("key found or created");

    HASHTABLE_MEMORY_FENCE_LOAD();

    key_value->data = value;

    LOG_DI("updating value to 0x%016x", value);

    if (created_new) {
        LOG_DI("it is a new key, updating flags and key");

        hashtable_key_value_flags_t flags = 0;

        LOG_DI("copying the key onto the key_value structure");

        // Get the destination pointer for the key
        if (key_size <= HASHTABLE_KEY_INLINE_MAX_LENGTH) {
            LOG_DI("key can be inline-ed", key_size);

            HASHTABLE_KEY_VALUE_SET_FLAG(flags, HASHTABLE_KEY_VALUE_FLAG_KEY_INLINE);

            ht_key = (hashtable_key_data_t *)&key_value->inline_key.data;
            strncpy((char*)ht_key, key, key_size);
        } else {
            LOG_DI("key can't be inline-ed, max length for inlining %d", HASHTABLE_KEY_INLINE_MAX_LENGTH);

            // TODO: The keys must be stored in an append-only store because potentially a get operation can be affected
            //       by a delete, never happened so far (under very high contention) but it can potentially happen and
            //       needs to be fixed.
            ht_key = xalloc_alloc(key_size + 1);
            ((char*)ht_key)[key_size] = '\0';
            strncpy((char*)ht_key, key, key_size);

            key_value->external_key.data = ht_key;
            key_value->external_key.size = key_size;
        }

        // Set the FILLED flag
        HASHTABLE_KEY_VALUE_SET_FLAG(flags, HASHTABLE_KEY_VALUE_FLAG_FILLED);

        HASHTABLE_MEMORY_FENCE_STORE();

        key_value->flags = flags;

        LOG_DI("key_value->flags = %d", key_value->flags);
    }

    // The unlock will perform the memory fence for us
    spinlock_unlock(&half_hashes_chunk->write_lock);

    LOG_DI("unlocking half_hashes_chunk 0x%016x", half_hashes_chunk);

    return true;
}
