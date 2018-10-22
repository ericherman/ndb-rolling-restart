/*
 * ndb-rolling-restart
 * Copyright (C) 2018 Eric Herman <eric@freesa.org>
 *
 * This work is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This work is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include <assert.h>
#include <getopt.h>
#include <iostream>
#include <mgmapi/mgmapi.h>
#include <ndbapi/NdbApi.hpp>
#include <string.h>
#include <unistd.h>

using namespace std;

#define NDB_NORMAL_USER 0
#define Cerr cerr << __FILE__ << ":" << __LINE__ << ": "

struct ndb_connection_context_s {
    const char* connect_string;
    unsigned wait_seconds;
    unsigned wait_after_restart;
    Ndb_cluster_connection* connection;
    NdbMgmHandle ndb_mgm_handle; /* a ptr */
    struct ndb_mgm_cluster_state* cluster_state;
};

struct restart_node_status_s {
    int node_group;
    int node_id;
    bool was_restarted;
};

/* Global */
static int verbose_flag = 0;

/* Global */
static struct option long_options[] = {
    { "connection_string", required_argument, NULL, 'c' },
    { "wait_seconds", optional_argument, NULL, 'w' },
    { "verbose", optional_argument, &verbose_flag, 1 },
    { 0, 0, 0, 0 }
};

enum binary_search_result {
    binary_search_error = 0,
    binary_search_found = 1,
    binary_search_insert = 2
};
/*
	Search elements for a target.
	returns
		binary_search_found: target is an element of elements,
			target_index is set to the location
		binary_search_insert: target is NOT an element of elements,
			target_index is set to the index where one would need
			to insert target
		binary_search_error: elements is NULL, target_index is unchanged
*/

static binary_search_result binary_search(int* elements, uint32_t num_elements,
    int target, size_t* target_index)
{
    size_t local_target_index = 0;

    // no data at all
    if (elements == NULL) {
        return binary_search_error;
    }

    if (!target_index) {
        target_index = &local_target_index;
    }

    // empty array, or insert location should be initial element
    if (num_elements == 0 || target < elements[0]) {
        *target_index = 0;
        return binary_search_insert;
    }

    uint32_t span = num_elements;
    uint32_t mid = num_elements / 2;
    uint32_t large_half;
    while (span > 0) {

        if (target == elements[mid]) {
            *target_index = mid;
            return binary_search_found;
        }

        span = span / 2; // half the range left over
        large_half = span / 2 + (span % 2); // being clever. But this is ceil

        if (target < elements[mid]) {
            mid -= large_half;
        } else {
            mid += large_half;
        }
    }

    // target is not an element of elements, but we found the closest location
    if (mid == num_elements) { // after all other elements
        *target_index = num_elements;
    } else if (target < elements[mid]) {
        *target_index = mid; // displace, shift the rest right
    } else if (target > elements[mid]) {
        *target_index = mid + 1; // not sure if these two are both possible
    } else {
        assert(0); // cannot happen
    }

    // correctness checks:
    // 1. array has elements, and we should insert at the end, make sure the
    // last element is smaller than the new one
    if (num_elements > 0 && *target_index == num_elements) {
        assert(target > elements[num_elements - 1]);
    }
    // 2. array has no elements
    // (we already check this above, but left for completeness)
    if (num_elements == 0) {
        assert(*target_index == 0);
    }
    // 3. array has elements, and we should insert at the beginning
    if (num_elements > 0 && *target_index == 0) {
        // MUST be smaller, otherwise it would have been found if equal
        assert(target < elements[0]);
    }
    // 4. insert somewhere in the middle
    if (*target_index > 0 && *target_index < num_elements) {
        // insert shifts the rest right, MUST be smaller otherwise it would
        // have been found
        assert(target < elements[*target_index]);
        // element to the left is smaller
        assert(elements[*target_index - 1] < target);
    }

    return binary_search_insert;
}

static void close_ndb_connection(struct ndb_connection_context_s* ndb_ctx)
{
    assert(ndb_ctx);

    if (ndb_ctx->cluster_state) {
        free((void*)ndb_ctx->cluster_state);
        ndb_ctx->cluster_state = nullptr;
    }

    if (ndb_ctx->ndb_mgm_handle) {
        ndb_mgm_destroy_handle(&(ndb_ctx->ndb_mgm_handle));
        ndb_ctx->ndb_mgm_handle = nullptr;
    }

    if (ndb_ctx->connection) {
        delete (ndb_ctx->connection);
        ndb_ctx->connection = nullptr;
    }
}

static Ndb_cluster_connection* ndb_connect(const char* connect_string,
    unsigned wait_seconds)
{
    Ndb_cluster_connection* cluster_connection;

    cluster_connection = new Ndb_cluster_connection(connect_string);
    if (!cluster_connection) {
        Cerr << "new Ndb_cluster_connection() returned 0" << endl;
        return nullptr;
    }

    int no_retries = 10;
    int retry_delay_in_seconds = wait_seconds;
    int verbose = 1;
    cluster_connection->connect(no_retries, retry_delay_in_seconds, verbose);

    int before_wait = wait_seconds;
    int after_wait = wait_seconds;
    if (cluster_connection->wait_until_ready(before_wait, after_wait) < 0) {
        Cerr << "Cluster was not ready within "
             << wait_seconds << " seconds" << endl;
        delete (cluster_connection);
        return nullptr;
    }
    return cluster_connection;
}

static int init_ndb_connection(struct ndb_connection_context_s* ndb_ctx)
{
    assert(ndb_ctx);

    ndb_ctx->connection = ndb_connect(ndb_ctx->connect_string,
        ndb_ctx->wait_seconds);
    if (!ndb_ctx->connection) {
        return 1;
    }

    ndb_ctx->ndb_mgm_handle = ndb_mgm_create_handle();
    if (!ndb_ctx->ndb_mgm_handle) {
        Cerr << "Error: ndb_mgm_create_handle returned null?" << endl;
        close_ndb_connection(ndb_ctx);
        return 1;
    }

    int no_retries = 10;
    int retry_delay_secs = 3;
    int verbose = 1;

    int ret = ndb_mgm_connect(ndb_ctx->ndb_mgm_handle, no_retries,
        retry_delay_secs, verbose);

    if (ret != 0) {
        Cerr "ndb_mgm_get_latest_error: "
            << ndb_mgm_get_latest_error_msg(ndb_ctx->ndb_mgm_handle)
            << endl;
        close_ndb_connection(ndb_ctx);
        return 1;
    }

    ndb_mgm_node_type node_types[2] = {
        /* NDB_MGM_NODE_TYPE_MGM, */ /* SKIP management server node */
        NDB_MGM_NODE_TYPE_NDB, /* database node, what we actually want */
        NDB_MGM_NODE_TYPE_UNKNOWN /* weird */
    };

    ndb_ctx->cluster_state = ndb_mgm_get_status2(ndb_ctx->ndb_mgm_handle,
        node_types);

    if (!ndb_ctx->cluster_state) {
        Cerr << "ndb_mgm_get_status2 returned null?" << endl;
        close_ndb_connection(ndb_ctx);
        return 1;
    }

    return 0;
}

static const string get_ndb_mgm_dump_state(NdbMgmHandle ndb_mgm_handle,
    struct ndb_mgm_node_state* node_state)
{
    assert(ndb_mgm_handle);
    assert(node_state);

    int arg_count = 1;
    int args[1] = { 1000 };
    struct ndb_mgm_reply reply;
    reply.return_code = 0;
    int node_id = node_state->node_id;
    int rv;

    rv = ndb_mgm_dump_state(ndb_mgm_handle, node_id, args, arg_count, &reply);
    if (rv == -1) {
        return "error: Could not dump state";
    }

    if (reply.return_code != 0) {
        return reply.message;
    }

    return "ok";
}

static int get_online_node_count(struct ndb_mgm_cluster_state* cluster_state)
{
    int online_nodes = 0;

    assert(cluster_state);
    for (int i = 0; i < cluster_state->no_of_nodes; ++i) {
        struct ndb_mgm_node_state* node_state;
        node_state = &(cluster_state->node_states[i]);
        if (node_state->node_status == NDB_MGM_NODE_STATUS_STARTED) {
            ++online_nodes;
        }
    }

    return online_nodes;
}

static void sleep_reconnect(struct ndb_connection_context_s* ndb_ctx)
{
    close_ndb_connection(ndb_ctx);
    cout << "sleep(" << ndb_ctx->wait_seconds << ")" << endl;
    sleep(ndb_ctx->wait_seconds);
    int err = init_ndb_connection(ndb_ctx);
    if (err) {
        Cerr << "could not reconnect to ndb" << endl;
    }
}

static int loop_wait_until_ready(struct ndb_connection_context_s* ndb_ctx,
    int node_id)
{

    assert(ndb_ctx->connection);

    int cnt = 1;
    int nodes[1] = { node_id };

    int ret = -1;
    while (ret == -1) {
        cout << "wait_until_ready node " << nodes[0]
             << " timeout: " << ndb_ctx->wait_seconds << endl;
        ret = ndb_ctx->connection->wait_until_ready(nodes, cnt,
            ndb_ctx->wait_seconds);
        if (ret <= -1) {
            Cerr << "ndb_mgm_restart4 returned error: " << ret << endl;
            sleep_reconnect(ndb_ctx);
        }
    }
    return 0;
}

static int restart_node(struct ndb_connection_context_s* ndb_ctx, int node_id)
{
    assert(ndb_ctx);

    int ret = 0;
    int disconnect = 0;
    int cnt = 1;
    int nodes[1] = { node_id };
    int initial = 0;
    int nostart = 0;
    int abort = 0;
    int force = 0;

    cout << "ndb_mgm_restart4 node " << nodes[0] << endl;

    loop_wait_until_ready(ndb_ctx, nodes[0]);

    ret = -1;
    while (ret <= 0) {
        ret = ndb_mgm_restart4(ndb_ctx->ndb_mgm_handle, cnt, nodes, initial,
            nostart, abort, force, &disconnect);
        if (ret <= 0) {
            cout << __FILE__ << ":" << __LINE__
                 << ": ndb_mgm_restart4 node " << nodes[0]
                 << " returned error: " << ret << endl;
            sleep_reconnect(ndb_ctx);
        }
    }

    if (disconnect) {
        sleep_reconnect(ndb_ctx);
    }

    if (ndb_ctx->wait_after_restart) {
        loop_wait_until_ready(ndb_ctx, nodes[0]);
    }

    cout << "restart node " << nodes[0] << " complete" << endl;
    return 0;
}

static struct restart_node_status_s* get_node_restarts(
    struct ndb_mgm_cluster_state* cluster_state, size_t* len)
{
    assert(cluster_state);
    assert(len);
    *len = 0;

    uint32_t number_of_nodes = cluster_state->no_of_nodes;
    if (number_of_nodes < 1) {
        Cerr << "cluster_state->no_of_nodes == " << number_of_nodes
             << " ?" << endl;
        return nullptr;
    }

    size_t size = sizeof(struct restart_node_status_s) * number_of_nodes;
    struct restart_node_status_s* node_restarts;
    node_restarts = (struct restart_node_status_s*)malloc(size);
    if (!node_restarts) {
        Cerr << "could not allocate " << size << " bytes?" << endl;
        return nullptr;
    }
    *len = number_of_nodes;

    for (uint32_t i = 0; i < number_of_nodes; ++i) {
        struct ndb_mgm_node_state* node_state;
        node_state = &(cluster_state->node_states[i]);
        node_restarts[i].node_group = node_state->node_group;
        node_restarts[i].node_id = node_state->node_id;
        node_restarts[i].was_restarted = 0;
    }

    // group them so they alternate by groups, lowest group id first

    // there can't be more groups than nodes
    int group_ids[number_of_nodes];
    uint32_t group_count = 0;

    // create a sorted array of group ids without duplicates
    size_t target_index;
    for (uint32_t i = 0; i < number_of_nodes; i++) {
        binary_search_result search_result = binary_search(group_ids,
            group_count, node_restarts[i].node_group, &target_index);
        if (search_result == binary_search_insert) {
            // we have enough space for sure, so we can move all current
            // elements over without writing past the end of the array
            // this way we end up with a sorted array (ascending)
            uint32_t elements_to_move = group_count - target_index;
            memmove((group_ids + target_index + 1), (group_ids + target_index),
                (sizeof(int) * elements_to_move));
            group_ids[target_index] = node_restarts[i].node_group;
            group_count++;
        }
        // already in the array
    }

    // now "take" nodes in turn from each of the groups
    // the convenient way is to start at node 0,
    // find a node from group 0 and swap them
    // then got to node 1 and to get a node from group 1 etc,
    //     looping the groups when at the end
    // if there is no node left from a group,
    // then remove that group from the list and keep going
    uint32_t update_node_index = 0, current_node_index = 0, current_group_index = 0;
    restart_node_status_s temp;
    while (update_node_index < number_of_nodes) {

        current_node_index = update_node_index;
        while (node_restarts[current_node_index].node_group
            != group_ids[current_group_index]) {
            current_node_index++;

            // if we reach the end without finding any,
            // then this group is exhausted.
            // remove this group from the list,
            // update the group count
            // and ensure current_group_index is within bounds
            if (current_node_index == number_of_nodes) {

                // move 1 left
                uint32_t elements_to_move = group_count - current_group_index - 1;

                memmove((group_ids + current_group_index),
                    (group_ids + current_group_index + 1),
                    (sizeof(uint32_t) * elements_to_move));
                group_count--;

                // in case it was at the last one, point back at 0
                current_group_index %= group_count;
                // start search over at the beginning
                current_node_index = update_node_index;
            }
        }

        temp = node_restarts[update_node_index];
        node_restarts[update_node_index] = node_restarts[current_node_index];
        node_restarts[current_node_index] = temp;

        update_node_index++;
        current_group_index = (current_group_index + 1) % group_count;
    }

    return node_restarts;
}

static void report_cluster_state(struct ndb_connection_context_s* ndb_ctx)
{
    assert(ndb_ctx);
    assert(ndb_ctx->connection);
    assert(ndb_ctx->cluster_state);

    const char* cluster_name = ndb_ctx->connection->get_system_name();
    cout << "cluster_name: " << cluster_name << endl;
    cout << "cluster_state->no_of_nodes: "
         << ndb_ctx->cluster_state->no_of_nodes << endl;

    for (int i = 0; i < ndb_ctx->cluster_state->no_of_nodes; ++i) {

        struct ndb_mgm_node_state* node_state;
        node_state = &(ndb_ctx->cluster_state->node_states[i]);

        cout << "node_id: " << node_state->node_id << " ("
             << ndb_mgm_get_node_type_string(node_state->node_type) << ")"
             << endl
             << "\tstatus: " << node_state->node_status << " ("
             << ndb_mgm_get_node_status_string(node_state->node_status)
             << ")" << endl;

        if ((node_state->node_type == NDB_MGM_NODE_TYPE_NDB)
            && (node_state->node_status == NDB_MGM_NODE_STATUS_STARTING)) {
            cout << "\tstart_phase: " << node_state->start_phase
                 << endl;
        }

        cout << "\tdynamic_id: " << node_state->dynamic_id << endl
             << "\tnode_group: " << node_state->node_group << endl
             << "\tversion: " << node_state->version << endl
             << "\tmysql_version: " << node_state->mysql_version << endl
             << "\tconnect_count: " << node_state->connect_count << endl
             << "\tconnect_address: " << node_state->connect_address << endl
             << "\tndb_mgm_dump_state: "
             << get_ndb_mgm_dump_state(ndb_ctx->ndb_mgm_handle, node_state)
             << endl;
    }

    int online_nodes = get_online_node_count(ndb_ctx->cluster_state);

    int offline_nodes = (ndb_ctx->cluster_state->no_of_nodes - online_nodes);

    cout << "no_of_nodes: " << ndb_ctx->cluster_state->no_of_nodes << endl
         << "online_nodes: " << online_nodes << endl
         << "offline_nodes: " << offline_nodes << endl;
}

int main(int argc, char** argv)
{
    ndb_init();

    struct ndb_connection_context_s ndb_ctx;

    ndb_ctx.connect_string = "";
    ndb_ctx.wait_seconds = 30;

    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "c:w:", long_options, &option_index)) != -1) {

        switch (c) {
        case 0: {
            /* If this option set a flag, do nothing else now. */
            break;
        }
        case 'c': {
            ndb_ctx.connect_string = optarg;
            break;
        }
        case 'w': {

            char* temp;
            unsigned long wait_seconds_arg = strtoul(optarg, &temp, 10);

            if (optarg != temp && *temp == '\0') { // parsed the whole thing
                ndb_ctx.wait_seconds = wait_seconds_arg;
            }
            break;
        }
        default: {
            abort();
        }
        }
    }

    /* skipping wait after restart can be a big speed improvement, but
       failure to wait after restart can be fatal:
       https://pastebin.com/raw/1mxgb99s */
    ndb_ctx.wait_after_restart = 1;

    int err = init_ndb_connection(&ndb_ctx);
    if (err) {
        Cerr << "error connecting to ndb '" << ndb_ctx.connect_string << "'"
             << endl;
        return 1;
    }

    report_cluster_state(&ndb_ctx);

    struct restart_node_status_s* node_restarts;
    size_t number_of_nodes = 0;
    node_restarts = get_node_restarts(ndb_ctx.cluster_state, &number_of_nodes);
    if (!node_restarts) {
        Cerr << "get_node_restarts returned NULL" << endl;
        close_ndb_connection(&ndb_ctx);
        return 1;
    }

    unsigned restarted = 0;
    int last_group = -1;
    for (size_t i = 0; restarted < number_of_nodes; ++i) {
        if (i >= number_of_nodes) {
            i = 0;
            last_group = -1;
        }
        if ((node_restarts[i].node_group != last_group)
            && !node_restarts[i].was_restarted) {
            ++restarted;
            node_restarts[i].was_restarted = 1;
            last_group = node_restarts[i].node_group;
            restart_node(&ndb_ctx, node_restarts[i].node_id);
        }
    }
    free(node_restarts);

    report_cluster_state(&ndb_ctx);

    close_ndb_connection(&ndb_ctx);
    ndb_end(NDB_NORMAL_USER);
    return 0;
}
