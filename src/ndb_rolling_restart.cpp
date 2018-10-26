/*
 * ndb_rolling_restart
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

#include "ndb_rolling_restart.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <thread>

using namespace std;

#define Cerr cerr << __FILE__ << ":" << __LINE__ << ": "

void close_ndb_connection(ndb_connection_context_s& ndb_ctx)
{
    if (ndb_ctx.cluster_state) {
        free((void*)ndb_ctx.cluster_state);
        ndb_ctx.cluster_state = nullptr;
    }

    if (ndb_ctx.ndb_mgm_handle) {
        ndb_mgm_destroy_handle(&(ndb_ctx.ndb_mgm_handle));
        ndb_ctx.ndb_mgm_handle = nullptr;
    }

    if (ndb_ctx.connection) {
        delete (ndb_ctx.connection);
        ndb_ctx.connection = nullptr;
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

int init_ndb_connection(ndb_connection_context_s& ndb_ctx)
{
    ndb_ctx.connection = ndb_connect(ndb_ctx.connect_string.c_str(),
        ndb_ctx.wait_seconds);
    if (!ndb_ctx.connection) {
        return 1;
    }

    ndb_ctx.ndb_mgm_handle = ndb_mgm_create_handle();
    if (!ndb_ctx.ndb_mgm_handle) {
        Cerr << "Error: ndb_mgm_create_handle returned null?" << endl;
        close_ndb_connection(ndb_ctx);
        return 1;
    }

    int no_retries = 10;
    int retry_delay_secs = 3;
    int verbose = 1;

    int ret = ndb_mgm_connect(ndb_ctx.ndb_mgm_handle, no_retries,
        retry_delay_secs, verbose);

    if (ret != 0) {
        Cerr "ndb_mgm_get_latest_error: "
            << ndb_mgm_get_latest_error_msg(ndb_ctx.ndb_mgm_handle)
            << endl;
        close_ndb_connection(ndb_ctx);
        return 1;
    }

    ndb_mgm_node_type node_types[2] = {
        /* NDB_MGM_NODE_TYPE_MGM, */ /* SKIP management server node */
        NDB_MGM_NODE_TYPE_NDB, /* database node, what we actually want */
        NDB_MGM_NODE_TYPE_UNKNOWN /* weird */
    };

    ndb_ctx.cluster_state = ndb_mgm_get_status2(ndb_ctx.ndb_mgm_handle,
        node_types);

    if (!ndb_ctx.cluster_state) {
        Cerr << "ndb_mgm_get_status2 returned null?" << endl;
        close_ndb_connection(ndb_ctx);
        return 1;
    }

    return 0;
}

static const string get_ndb_mgm_dump_state(NdbMgmHandle ndb_mgm_handle,
                                           ndb_mgm_node_state node_state)
{
    assert(ndb_mgm_handle);

    int arg_count = 1;
    int args[1] = { 1000 };
    ndb_mgm_reply reply;
    reply.return_code = 0;
    int node_id = node_state.node_id;
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

static int get_online_node_count(ndb_mgm_cluster_state* cluster_state)
{
    int online_nodes = 0;

    assert(cluster_state);
    for (int i = 0; i < cluster_state->no_of_nodes; ++i) {
        auto node_state = &(cluster_state->node_states[i]);
        if (node_state->node_status == NDB_MGM_NODE_STATUS_STARTED) {
            ++online_nodes;
        }
    }

    return online_nodes;
    // this has some off-by-one error returning node_states+1
    // return std::count_if(
    //      cluster_state->node_states,
    //      cluster_state->node_states +
    //	    cluster_state->no_of_nodes*sizeof(ndb_mgm_node_state),
    //      [](const ndb_mgm_node_state& state) {
    //              return state.node_status == NDB_MGM_NODE_STATUS_STARTED; });
}

static void sleep_reconnect(ndb_connection_context_s& ndb_ctx)
{
    close_ndb_connection(ndb_ctx);
    cout << "sleep(" << ndb_ctx.wait_seconds << ")" << endl;
    this_thread::sleep_for(chrono::seconds(ndb_ctx.wait_seconds));
    int err = init_ndb_connection(ndb_ctx);
    if (err) {
        Cerr << "could not reconnect to ndb" << endl;
    }
}

static int loop_wait_until_ready(ndb_connection_context_s& ndb_ctx,
    int node_id)
{
    assert(ndb_ctx.connection);

    int cnt = 1;
    int nodes[1] = { node_id };

    int ret = -1;
    while (ret == -1) {
        cout << "wait_until_ready node " << nodes[0]
             << " timeout: " << ndb_ctx.wait_seconds << endl;
        ret = ndb_ctx.connection->wait_until_ready(nodes, cnt,
            ndb_ctx.wait_seconds);
        if (ret <= -1) {
            Cerr << "ndb_mgm_restart4 returned error: " << ret << endl;
            sleep_reconnect(ndb_ctx);
        }
    }
    return 0;
}

int restart_node(ndb_connection_context_s& ndb_ctx, int node_id)
{
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
        ret = ndb_mgm_restart4(ndb_ctx.ndb_mgm_handle, cnt, nodes, initial,
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

    if (ndb_ctx.wait_after_restart) {
        loop_wait_until_ready(ndb_ctx, nodes[0]);
    }

    cout << "restart node " << nodes[0] << " complete" << endl;
    return 0;
}

void sort_node_restarts(std::vector<restart_node_status_s>& nodes)
{
    //Build a multimap so that different index fall into different group
    //and then fetch one by one from each group
    assert(nodes.size());
    std::sort(nodes.begin(), nodes.end(),
         [](const restart_node_status_s & n1, const restart_node_status_s & n2)
         { return n1.node_id < n2.node_id; });
    std::multimap<decltype(nodes.front().node_group), size_t> indexes;
    
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        indexes.emplace(nodes[i].node_group, i);
    }

    std::vector<restart_node_status_s> sorted_nodes;
    while(!indexes.empty())
    {
        for (auto it = indexes.begin(); it != indexes.end();)
        {
            auto gid = it->first;
            sorted_nodes.emplace_back(std::move(nodes[it->second]));
            indexes.erase(it);
            it = indexes.upper_bound(gid);
        }
    }
    sorted_nodes.swap(nodes);
}

vector<restart_node_status_s> get_node_restarts(ndb_mgm_cluster_state* cluster_state,
                                                size_t number_of_nodes)
{    
    assert(cluster_state);
    assert(number_of_nodes);

    vector<restart_node_status_s> node_restarts;
    for (size_t i = 0; i < number_of_nodes; ++i) {
        node_restarts.emplace_back(restart_node_status_s{cluster_state->node_states[i].node_id, 
                                                         cluster_state->node_states[i].node_group, 
                                                         false});
    }
    return node_restarts;
}

void report_cluster_state(ndb_connection_context_s& ndb_ctx)
{
    assert(ndb_ctx.connection);
    assert(ndb_ctx.cluster_state);

    auto cluster_name = ndb_ctx.connection->get_system_name();
    cout << "cluster_name: " << cluster_name << endl;
    cout << "cluster_state->no_of_nodes: "
         << ndb_ctx.cluster_state->no_of_nodes << endl;

    for (int i = 0; i < ndb_ctx.cluster_state->no_of_nodes; ++i) {

        auto node_state = ndb_ctx.cluster_state->node_states[i];

        cout << "node_id: " << node_state.node_id << " ("
             << ndb_mgm_get_node_type_string(node_state.node_type) << ")"
             << endl
             << "\tstatus: " << node_state.node_status << " ("
             << ndb_mgm_get_node_status_string(node_state.node_status)
             << ")" << endl;

        if ((node_state.node_type == NDB_MGM_NODE_TYPE_NDB)
            && (node_state.node_status == NDB_MGM_NODE_STATUS_STARTING)) {
            cout << "\tstart_phase: " << node_state.start_phase
                 << endl;
        }

        cout << "\tdynamic_id: " << node_state.dynamic_id << endl
             << "\tnode_group: " << node_state.node_group << endl
             << "\tversion: " << node_state.version << endl
             << "\tmysql_version: " << node_state.mysql_version << endl
             << "\tconnect_count: " << node_state.connect_count << endl
             << "\tconnect_address: " << node_state.connect_address << endl
             << "\tndb_mgm_dump_state: "
             << get_ndb_mgm_dump_state(ndb_ctx.ndb_mgm_handle, node_state)
             << endl;
    }

    int online_nodes = get_online_node_count(ndb_ctx.cluster_state);

    int offline_nodes = (ndb_ctx.cluster_state->no_of_nodes - online_nodes);

    cout << "no_of_nodes: " << ndb_ctx.cluster_state->no_of_nodes << endl
         << "online_nodes: " << online_nodes << endl
         << "offline_nodes: " << offline_nodes << endl;
}

int ndb_rolling_restart(ndb_connection_context_s& ndb_ctx)
{
    ndb_init();

    int err = init_ndb_connection(ndb_ctx);
    if (err) {
        Cerr << "error connecting to ndb '" << ndb_ctx.connect_string << "'"
             << endl;
        return 1;
    }

    report_cluster_state(ndb_ctx);

    if (ndb_ctx.cluster_state->no_of_nodes < 1) {
        Cerr << "cluster_state->no_of_nodes == "
             << ndb_ctx.cluster_state->no_of_nodes << " ?" << endl;
        close_ndb_connection(ndb_ctx);
        return EXIT_FAILURE;
    }

    auto number_of_nodes = (size_t)ndb_ctx.cluster_state->no_of_nodes;
    auto node_restarts = get_node_restarts(ndb_ctx.cluster_state, number_of_nodes);

    assert(number_of_nodes == node_restarts.size());

    sort_node_restarts(node_restarts);

    size_t restarted = 0;
    int last_group = -1;
    for (size_t i = 0; restarted < number_of_nodes; ++i) {
        if (i >= number_of_nodes) {
            i = 0;
            last_group = -1;
        }
        if ((node_restarts[i].node_group != last_group)
            && !node_restarts[i].was_restarted) {
            ++restarted;
            node_restarts[i].was_restarted = true;
            last_group = node_restarts[i].node_group;
            restart_node(ndb_ctx, node_restarts[i].node_id);
        }
    }

    report_cluster_state(ndb_ctx);

    close_ndb_connection(ndb_ctx);
    ndb_end(NDB_NORMAL_USER);
    return 0;
}
