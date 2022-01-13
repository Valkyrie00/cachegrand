/**
 * Copyright (C) 2020-2021 Daniele Salvatore Albano
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 **/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <string.h>
#include <fatal.h>

#include "misc.h"
#include "exttypes.h"
#include "spinlock.h"
#include "data_structures/hashtable/mcmp/hashtable.h"
#include "config.h"
#include "log/log.h"
#include "fiber.h"
#include "network/protocol/network_protocol.h"
#include "network/io/network_io_common.h"
#include "network/channel/network_channel.h"
#include "storage/io/storage_io_common.h"
#include "storage/channel/storage_channel.h"
#include "worker/worker_stats.h"
#include "worker/worker_context.h"
#include "worker/worker.h"
#include "worker/worker_op.h"
#include "fiber_scheduler.h"
#include "worker/network/worker_network.h"
#include "protocol/redis/protocol_redis_reader.h"
#include "network/protocol/redis/network_protocol_redis.h"

#include "worker_network_op.h"

#define TAG "worker_network_op"

// Network operations
worker_op_network_channel_new_fp_t* worker_op_network_channel_new;
worker_op_network_channel_multi_new_fp_t* worker_op_network_channel_multi_new;
worker_op_network_channel_multi_get_fp_t* worker_op_network_channel_multi_get;
worker_op_network_channel_size_fp_t* worker_op_network_channel_size;
worker_op_network_channel_free_fp_t* worker_op_network_channel_free;
worker_op_network_accept_fp_t* worker_op_network_accept;
worker_op_network_receive_fp_t* worker_op_network_receive;
worker_op_network_send_fp_t* worker_op_network_send;
worker_op_network_close_fp_t* worker_op_network_close;

void worker_network_new_client_fiber_entrypoint(
        void *user_data) {
    worker_context_t *context = worker_context_get();
    worker_stats_t *stats = worker_stats_get();

    network_channel_t *new_channel = user_data;

    stats->network.total.active_connections++;
    stats->network.total.accepted_connections++;
    stats->network.per_second.accepted_connections++;

    // Should not access the listener_channel directly
    switch (new_channel->protocol) {
        default:
            // This can't really happen
            FATAL(
                    TAG,
                    "[FD:%5d][ACCEPT] Channel unknown protocol <%d>",
                    new_channel->fd,
                    new_channel->protocol);
            break;

        case NETWORK_PROTOCOLS_REDIS:
            network_protocol_redis_accept(
                    new_channel);
            break;
    }

    // Close the connection
    worker_network_close(new_channel, true);

    // Updates the worker stats
    stats->network.total.active_connections--;

    fiber_scheduler_terminate_current_fiber();
}

void worker_network_listeners_fiber_entrypoint(
        void* user_data) {
    worker_context_t *worker_context = worker_context_get();
    network_channel_t *new_channel = NULL;
    network_channel_t *listener_channel = user_data;

    while(!worker_should_terminate(worker_context)) {
        if ((new_channel = worker_op_network_accept(
                listener_channel)) == NULL) {
            LOG_W(
                    TAG,
                    "[FD:%5d][ACCEPT] Listener <%s> failed to accept a new connection",
                    listener_channel->fd,
                    listener_channel->address.str);

            continue;
        }

        LOG_V(
                TAG,
                "[FD:%5d][ACCEPT] Listener <%s> accepting new connection from <%s>",
                listener_channel->fd,
                listener_channel->address.str,
                new_channel->address.str);

        fiber_scheduler_new_fiber(
                "worker-listener-client",
                strlen("worker-listener-client"),
                worker_network_new_client_fiber_entrypoint,
                (void *)new_channel);
    }

    // Switch back to the scheduler, as the lister has been closed this fiber will never be invoked and will get freed
    fiber_scheduler_switch_back();
}
