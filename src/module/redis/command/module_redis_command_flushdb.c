/**
 * Copyright (C) 2018-2022 Daniele Salvatore Albano
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 **/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>

#include "misc.h"
#include "exttypes.h"
#include "log/log.h"
#include "clock.h"
#include "spinlock.h"
#include "data_structures/small_circular_queue/small_circular_queue.h"
#include "data_structures/double_linked_list/double_linked_list.h"
#include "data_structures/hashtable/mcmp/hashtable.h"
#include "data_structures/hashtable/spsc/hashtable_spsc.h"
#include "protocol/redis/protocol_redis.h"
#include "protocol/redis/protocol_redis_reader.h"
#include "protocol/redis/protocol_redis_writer.h"
#include "module/module.h"
#include "network/io/network_io_common.h"
#include "config.h"
#include "fiber.h"
#include "network/channel/network_channel.h"
#include "storage/io/storage_io_common.h"
#include "storage/channel/storage_channel.h"
#include "storage/db/storage_db.h"
#include "module/redis/module_redis.h"
#include "module/redis/module_redis_connection.h"
#include "module/redis/module_redis_command.h"
#include "network/network.h"
#include "worker/worker_stats.h"
#include "worker/worker_context.h"

#define TAG "module_redis_command_flushdb"

MODULE_REDIS_COMMAND_FUNCPTR_COMMAND_END(flushdb) {
    module_redis_command_flushdb_context_t *context = connection_context->command.context;

    if (context->condition.value.async_async.has_token) {
        module_redis_connection_error_message_printf_noncritical(
                connection_context,
                "ERR the async flushdb is currently unsupported");
        return true;
    }

    if (!storage_db_op_flush_sync(connection_context->db)) {
        module_redis_connection_error_message_printf_noncritical(
                connection_context,
                "ERR Failed to flush the database");
        return true;
    }

    module_redis_connection_send_ok(connection_context);

    return true;
}