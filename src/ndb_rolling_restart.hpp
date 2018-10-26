/*
 * ndb_rolling_restart.hpp
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

#ifndef NDB_ROLLING_RESTART_H
#define NDB_ROLLING_RESTART_H 1

#include <mgmapi/mgmapi.h> // typedef struct ndb_mgm_handle * NdbMgmHandle;
#include <ndbapi/NdbApi.hpp> // class Ndb_cluster_connection
#include <string>
#include <vector>

#define NDB_NORMAL_USER 0

struct ndb_connection_context_s {
    std::string connect_string;
    unsigned wait_seconds = 30;
    /* skipping wait after restart can be a big speed improvement, but
       failure to wait after restart can be fatal:
       https://pastebin.com/raw/1mxgb99s */
    bool wait_after_restart = true;
    Ndb_cluster_connection* connection;
    NdbMgmHandle ndb_mgm_handle; /* a ptr */
    ndb_mgm_cluster_state* cluster_state;
};

struct restart_node_status_s {
    int node_id;
    int node_group;
    bool was_restarted;
};

void close_ndb_connection(ndb_connection_context_s& ndb_ctx);

int init_ndb_connection(ndb_connection_context_s& ndb_ctx);

int restart_node(ndb_connection_context_s& ndb_ctx, int node_id);

void sort_node_restarts(std::vector<restart_node_status_s>& nodes);

std::vector<restart_node_status_s> get_node_restarts(ndb_mgm_cluster_state* cluster_state,
                                                     size_t number_of_nodes);

void report_cluster_state(ndb_connection_context_s& ndb_ctx);

int ndb_rolling_restart(ndb_connection_context_s& ndb_ctx);

#endif /* NDB_ROLLING_RESTART_H */
