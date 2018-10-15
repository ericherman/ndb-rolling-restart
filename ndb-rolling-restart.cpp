#include <algorithm>
#include <arpa/inet.h>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <mgmapi/mgmapi.h>
#include <mgmapi/mgmapi_debug.h>
#include <mutex>
#include <ndbapi/NdbApi.hpp>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#define DBACC 0xF8
#define DBTUP 0xF9

using namespace std;

const string get_ndb_mgm_dump_state(NdbMgmHandle ndb_mgm_handle,
    struct ndb_mgm_node_state* node_state)
{
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

Ndb_cluster_connection* ndb_connect(const char* connect_string,
    unsigned wait_seconds)
{
    Ndb_cluster_connection* cluster_connection;

    cluster_connection = new Ndb_cluster_connection(connect_string);
    if (!cluster_connection) {
        cerr << "new Ndb_cluster_connection() returned 0" << endl;
        return nullptr;
    }

    cluster_connection->connect(2, 5, 1);

    if (cluster_connection->wait_until_ready(wait_seconds, 0) < 0) {
        cerr << "Cluster was not ready within "
             << wait_seconds << " seconds" << endl;
        return nullptr;
    }
    return cluster_connection;
}

int get_online_node_count(struct ndb_mgm_cluster_state* cluster_state)
{
    assert(cluster_state);

    int online_nodes = 0;

    for (int i = 0; i < cluster_state->no_of_nodes; ++i) {
        struct ndb_mgm_node_state* node_state;
        node_state = &(cluster_state->node_states[i]);
        if (node_state->node_status == NDB_MGM_NODE_STATUS_STARTED) {
            ++online_nodes;
        }
    }

    return online_nodes;
}

int restart_node(Ndb_cluster_connection* cluster_connection,
    NdbMgmHandle ndb_mgm_handle,
    struct ndb_mgm_node_state* node_state,
    unsigned wait_seconds)
{
    assert(cluster_connection);
    assert(ndb_mgm_handle);
    assert(node_state);

    int ret = 0;
    int disconnect = 0;
    int cnt = 1;
    int nodes[1] = { node_state->node_id };
    int initial = 0;
    int nostart = 0;
    int abort = 0;
    int force = 0;

    cout << "ndb_mgm_restart4 node " << nodes[0] << endl;
    ret = ndb_mgm_restart4(ndb_mgm_handle, cnt, nodes, initial, nostart,
        abort, force, &disconnect);
    if (ret <= 0) {
        cerr << "ndb_mgm_restart4 node " << nodes[0]
             << " returned error: " << ret << endl;
        return 1;
    }
    if (disconnect) {
        cerr << "ndb_mgm_restart4 node " << nodes[0]
             << " returned disconnect: " << disconnect << "?" << endl;
        return 1;
    }

    ret = -1;
    while (ret == -1) {
        cout << "wait_until_ready node " << nodes[0]
             << " timeout: " << wait_seconds << endl;
        ret = cluster_connection->wait_until_ready(nodes, cnt, wait_seconds);
        if (ret < -1) {
            cerr << "ndb_mgm_restart4 returned error: " << ret << endl;
            return 1;
        } else {
            cout << "sleep(" << wait_seconds << ")" << endl;
            sleep(wait_seconds);
        }
    }

    cout << "restart node " << nodes[0] << " complete" << endl;
    return 0;
}

void report_cluster_state(NdbMgmHandle ndb_mgm_handle,
    ndb_mgm_node_type* node_types)
{
    assert(ndb_mgm_handle);
    assert(node_types);

    struct ndb_mgm_cluster_state* cluster_state;
    cluster_state = ndb_mgm_get_status2(ndb_mgm_handle, node_types);

    cout << "cluster_state->no_of_nodes: " << cluster_state->no_of_nodes
         << endl;

    for (int i = 0; i < cluster_state->no_of_nodes; ++i) {

        struct ndb_mgm_node_state* node_state;
        node_state = &(cluster_state->node_states[i]);

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
             << get_ndb_mgm_dump_state(ndb_mgm_handle, node_state)
             << endl;
    }

    int online_nodes = get_online_node_count(cluster_state);

    int offline_nodes = (cluster_state->no_of_nodes - online_nodes);

    cout << "no_of_nodes: " << cluster_state->no_of_nodes << endl
         << "online_nodes: " << online_nodes << endl
         << "offline_nodes: " << offline_nodes << endl;

    free((void*)cluster_state);
}

int main(int argc, char** argv)
{
    const char* connect_string;
    unsigned wait_seconds = 30;

    connect_string = argc > 1 ? argv[1] : "";
    ndb_init();

    Ndb_cluster_connection* cluster_connection;

    cluster_connection = ndb_connect(connect_string, wait_seconds);
    if (!cluster_connection) {
        return 1;
    }

    const char* cluster_name = cluster_connection->get_system_name();
    cout << "cluster_name: " << cluster_name << endl;

    NdbMgmHandle ndb_mgm_handle = ndb_mgm_create_handle();
    if (ndb_mgm_connect(ndb_mgm_handle, 3, 1, 1) != 0) {
        cout << "Error: "
             << ndb_mgm_get_latest_error_msg(ndb_mgm_handle)
             << endl;
        return 1;
    }

    ndb_mgm_node_type node_types[2] = {
        /* NDB_MGM_NODE_TYPE_MGM, */ /* SKIP management server node */
        NDB_MGM_NODE_TYPE_NDB, /* database node, what we actually want */
        NDB_MGM_NODE_TYPE_UNKNOWN /* weird */
    };

    report_cluster_state(ndb_mgm_handle, node_types);

    struct ndb_mgm_cluster_state* cluster_state;
    cluster_state = ndb_mgm_get_status2(ndb_mgm_handle, node_types);

    int online_nodes = get_online_node_count(cluster_state);
    int offline_nodes = (cluster_state->no_of_nodes - online_nodes);
    if (offline_nodes != 0) {
        cout << "nodes are off-line, aborting" << endl;
    }

    int number_of_nodes = cluster_state->no_of_nodes;
    int err = offline_nodes;
    for (int i = 0; !err && i < number_of_nodes; ++i) {
        struct ndb_mgm_node_state* node_state;
        node_state = &(cluster_state->node_states[i]);
        err = restart_node(cluster_connection, ndb_mgm_handle,
            node_state, wait_seconds);
    }

    cout << "finished";
    if (err) {
        cout << " with errors";
    }
    cout << endl
         << endl;

    report_cluster_state(ndb_mgm_handle, node_types);

    free((void*)cluster_state);
    ndb_mgm_destroy_handle(&ndb_mgm_handle);
    delete (cluster_connection);
    ndb_end(0);
}
