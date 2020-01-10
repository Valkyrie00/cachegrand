#include "catch.hpp"

#include <string.h>

#include "hashtable/hashtable.h"
#include "hashtable/hashtable_config.h"
#include "hashtable/hashtable_data.h"
#include "hashtable/hashtable_support.h"
#include "hashtable/hashtable_gc.h"
#include "hashtable/hashtable_op_get.h"
#include "hashtable/hashtable_op_set.h"

#include "test-hashtable.h"

TEST_CASE("hashtable_op_get.c", "[hashtable][hashtable_op_get]") {
    SECTION("hashtable_get") {
        hashtable_value_data_t value = 0;
        hashtable_bucket_index_t test_index_1 = test_key_1_hash % buckets_count_53;
        hashtable_bucket_index_t test_index_2 = test_key_2_hash % buckets_count_53;

        SECTION("not found - hashtable empty") {
            HASHTABLE_INIT(buckets_initial_count_5, false, {
                REQUIRE(!hashtable_get(
                        hashtable,
                        test_key_1,
                        test_key_1_len,
                        &value));
            })
        }

        SECTION("found - test key inline") {
            HASHTABLE_INIT(buckets_initial_count_5, false, {
                HASHTABLE_BUCKET_HASH_KEY_VALUE_SET_KEY_INLINE(
                        test_index_1,
                        test_key_1_hash,
                        test_key_1,
                        test_key_1_len,
                        test_value_1);

                REQUIRE(hashtable_get(
                        hashtable,
                        test_key_1,
                        test_key_1_len,
                        &value));
            })
        }

        SECTION("found - test key external") {
            HASHTABLE_INIT(buckets_initial_count_5, false, {
                HASHTABLE_BUCKET_HASH_KEY_VALUE_SET_KEY_EXTERNAL(
                        test_index_1,
                        test_key_1_hash,
                        test_key_1,
                        test_key_1_len,
                        test_value_1);

                REQUIRE(hashtable_get(
                        hashtable,
                        test_key_1,
                        test_key_1_len,
                        &value));
            })
        }

        SECTION("found - test multiple buckets with key inline") {
            HASHTABLE_INIT(buckets_initial_count_5, false, {
                HASHTABLE_BUCKET_HASH_KEY_VALUE_SET_KEY_INLINE(
                        test_index_1,
                        test_key_1_hash,
                        test_key_1,
                        test_key_1_len,
                        test_value_1);

                HASHTABLE_BUCKET_HASH_KEY_VALUE_SET_KEY_INLINE(
                        test_index_2,
                        test_key_2_hash,
                        test_key_2,
                        test_key_2_len,
                        test_value_2);

                REQUIRE(hashtable_get(
                        hashtable,
                        test_key_1,
                        test_key_1_len,
                        &value));

                REQUIRE(value == test_value_1);

                REQUIRE(hashtable_get(
                        hashtable,
                        test_key_2,
                        test_key_2_len,
                        &value));

                REQUIRE(value == test_value_2);
            })
        }

        SECTION("not found - test deleted flag") {
            HASHTABLE_INIT(buckets_initial_count_5, false, {
                HASHTABLE_BUCKET_HASH_KEY_VALUE_SET_KEY_INLINE(
                        test_index_1,
                        test_key_1_hash,
                        test_key_1,
                        test_key_1_len,
                        test_value_1);

                HASHTABLE_BUCKET_KEY_VALUE_SET_FLAG(
                        hashtable->ht_current->keys_values[test_index_1].flags,
                        HASHTABLE_BUCKET_KEY_VALUE_FLAG_DELETED);

                REQUIRE(!hashtable_get(
                        hashtable,
                        test_key_1,
                        test_key_1_len,
                        &value));
            })
        }

        SECTION("found - test neighborhood") {
            HASHTABLE_INIT(buckets_initial_count_5, false, {
                hashtable_bucket_index_t index_neighborhood_begin, index_neighborhood_end;

                hashtable_calculate_neighborhood_from_hash(
                        hashtable->ht_current->buckets_count,
                        test_key_1_hash,
                        &index_neighborhood_begin,
                        &index_neighborhood_end);

                HASHTABLE_BUCKET_HASH_KEY_VALUE_SET_KEY_INLINE(
                        index_neighborhood_begin,
                        test_key_1_hash,
                        test_key_1,
                        test_key_1_len,
                        test_value_1);

                REQUIRE(hashtable_get(
                        hashtable,
                        test_key_1,
                        test_key_1_len,
                        &value));

                REQUIRE(value == test_value_1);

                hashtable->ht_current->hashes[index_neighborhood_begin] = 0;

                HASHTABLE_BUCKET_HASH_KEY_VALUE_SET_KEY_INLINE(
                        index_neighborhood_end,
                        test_key_1_hash,
                        test_key_1,
                        test_key_1_len,
                        test_value_1);

                REQUIRE(hashtable_get(
                        hashtable,
                        test_key_1,
                        test_key_1_len,
                        &value));

                REQUIRE(value == test_value_1);
            })
        }

        SECTION("not found - test hash set but key_value not (edge case because of parallelism)") {
            HASHTABLE_INIT(buckets_initial_count_5, false, {
                hashtable->ht_current->hashes[test_index_1] = test_key_1_hash;

                REQUIRE(!hashtable_get(
                        hashtable,
                        test_key_1,
                        test_key_1_len,
                        &value));
            })
        }

        SECTION("found - single bucket - get key after delete with hash still in hashes (edge case because of parallelism)") {
            HASHTABLE_INIT(buckets_initial_count_5, false, {
                hashtable_bucket_index_t index_neighborhood_begin, index_neighborhood_end;

                hashtable_calculate_neighborhood_from_hash(
                        hashtable->ht_current->buckets_count,
                        test_key_1_hash,
                        &index_neighborhood_begin,
                        &index_neighborhood_end);

                HASHTABLE_BUCKET_HASH_KEY_VALUE_SET_KEY_INLINE(
                        index_neighborhood_begin,
                        test_key_1_hash,
                        test_key_1,
                        test_key_1_len,
                        test_value_1);

                HASHTABLE_BUCKET_KEY_VALUE_SET_FLAG(
                        hashtable->ht_current->keys_values[test_index_1].flags,
                        HASHTABLE_BUCKET_KEY_VALUE_FLAG_DELETED);

                HASHTABLE_BUCKET_HASH_KEY_VALUE_SET_KEY_INLINE(
                        index_neighborhood_end,
                        test_key_1_hash,
                        test_key_1,
                        test_key_1_len,
                        test_value_1);

                REQUIRE(hashtable_get(
                        hashtable,
                        test_key_1,
                        test_key_1_len,
                        &value));

                REQUIRE(value == test_value_1);
            })
        }
    }
}