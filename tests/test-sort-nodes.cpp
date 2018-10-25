#include <stdlib.h>

#include "echeck.h"
#include "ndb_rolling_restart.hpp"

#define STATIC_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

int test_node_sorting(struct restart_node_status_s* nodes, size_t nodes_len,
    struct restart_node_status_s* expected_nodes, int verbose)
{
    if (verbose) {
        printf("Nodes in input order:\n");
        for (size_t i = 0; i < nodes_len; ++i) {
            printf("Node[%02lu]: node_id:%d, group = %d\n", (unsigned long)i,
                nodes[i].node_id, nodes[i].node_group);
        }
    }

    sort_node_restarts(nodes, nodes_len);

    if (verbose) {
        printf("Nodes in output order:\n");
        for (size_t i = 0; i < nodes_len; ++i) {
            printf("Node[%02lu]: node_id:%d, group = %d\n", (unsigned long)i,
                nodes[i].node_id, nodes[i].node_group);
        }
    }

    int failures = 0;
    for (size_t i = 0; i < nodes_len; ++i) {
        char buf[80];
        sprintf(buf, "node[%lu].node_id", (unsigned long)i);
        failures += check_int_m(nodes[i].node_id, expected_nodes[i].node_id,
            buf);
        sprintf(buf, "node[%lu].node_group", (unsigned long)i);
        failures += check_int_m(nodes[i].node_group,
            expected_nodes[i].node_group, buf);
    }
    return failures;
}

int test_node_sorting_48(int verbose)
{
    struct restart_node_status_s nodes[] = {
        //
        { node_id : 47, node_group : 23, was_restarted : false }, //
        { node_id : 20, node_group : 9, was_restarted : false }, //
        { node_id : 39, node_group : 19, was_restarted : false }, //
        { node_id : 26, node_group : 12, was_restarted : false }, //
        { node_id : 6, node_group : 2, was_restarted : false }, //
        { node_id : 37, node_group : 18, was_restarted : false }, //
        { node_id : 7, node_group : 3, was_restarted : false }, //
        { node_id : 29, node_group : 14, was_restarted : false }, //
        { node_id : 40, node_group : 19, was_restarted : false }, //
        { node_id : 25, node_group : 12, was_restarted : false }, //
        { node_id : 36, node_group : 17, was_restarted : false }, //
        { node_id : 1, node_group : 0, was_restarted : false }, //
        { node_id : 8, node_group : 3, was_restarted : false }, //
        { node_id : 9, node_group : 4, was_restarted : false }, //
        { node_id : 30, node_group : 14, was_restarted : false }, //
        { node_id : 14, node_group : 6, was_restarted : false }, //
        { node_id : 31, node_group : 15, was_restarted : false }, //
        { node_id : 13, node_group : 6, was_restarted : false }, //
        { node_id : 21, node_group : 10, was_restarted : false }, //
        { node_id : 16, node_group : 7, was_restarted : false }, //
        { node_id : 42, node_group : 20, was_restarted : false }, //
        { node_id : 3, node_group : 1, was_restarted : false }, //
        { node_id : 24, node_group : 11, was_restarted : false }, //
        { node_id : 5, node_group : 2, was_restarted : false }, //
        { node_id : 2, node_group : 0, was_restarted : false }, //
        { node_id : 18, node_group : 8, was_restarted : false }, //
        { node_id : 11, node_group : 5, was_restarted : false }, //
        { node_id : 27, node_group : 13, was_restarted : false }, //
        { node_id : 33, node_group : 16, was_restarted : false }, //
        { node_id : 44, node_group : 21, was_restarted : false }, //
        { node_id : 38, node_group : 18, was_restarted : false }, //
        { node_id : 15, node_group : 7, was_restarted : false }, //
        { node_id : 10, node_group : 4, was_restarted : false }, //
        { node_id : 32, node_group : 15, was_restarted : false }, //
        { node_id : 4, node_group : 1, was_restarted : false }, //
        { node_id : 46, node_group : 22, was_restarted : false }, //
        { node_id : 48, node_group : 23, was_restarted : false }, //
        { node_id : 43, node_group : 21, was_restarted : false }, //
        { node_id : 28, node_group : 13, was_restarted : false }, //
        { node_id : 41, node_group : 20, was_restarted : false }, //
        { node_id : 35, node_group : 17, was_restarted : false }, //
        { node_id : 22, node_group : 10, was_restarted : false }, //
        { node_id : 12, node_group : 5, was_restarted : false }, //
        { node_id : 19, node_group : 9, was_restarted : false }, //
        { node_id : 34, node_group : 16, was_restarted : false }, //
        { node_id : 45, node_group : 22, was_restarted : false }, //
        { node_id : 17, node_group : 8, was_restarted : false }, //
        { node_id : 23, node_group : 11, was_restarted : false } //
    };

    struct restart_node_status_s expected_nodes[] = {
        //
        { node_id : 2, node_group : 0, was_restarted : false }, //
        { node_id : 4, node_group : 1, was_restarted : false }, //
        { node_id : 6, node_group : 2, was_restarted : false }, //
        { node_id : 8, node_group : 3, was_restarted : false }, //
        { node_id : 10, node_group : 4, was_restarted : false }, //
        { node_id : 12, node_group : 5, was_restarted : false }, //
        { node_id : 14, node_group : 6, was_restarted : false }, //
        { node_id : 16, node_group : 7, was_restarted : false }, //
        { node_id : 18, node_group : 8, was_restarted : false }, //
        { node_id : 20, node_group : 9, was_restarted : false }, //
        { node_id : 22, node_group : 10, was_restarted : false }, //
        { node_id : 24, node_group : 11, was_restarted : false }, //
        { node_id : 26, node_group : 12, was_restarted : false }, //
        { node_id : 28, node_group : 13, was_restarted : false }, //
        { node_id : 30, node_group : 14, was_restarted : false }, //
        { node_id : 32, node_group : 15, was_restarted : false }, //
        { node_id : 34, node_group : 16, was_restarted : false }, //
        { node_id : 36, node_group : 17, was_restarted : false }, //
        { node_id : 38, node_group : 18, was_restarted : false }, //
        { node_id : 40, node_group : 19, was_restarted : false }, //
        { node_id : 42, node_group : 20, was_restarted : false }, //
        { node_id : 44, node_group : 21, was_restarted : false }, //
        { node_id : 46, node_group : 22, was_restarted : false }, //
        { node_id : 48, node_group : 23, was_restarted : false }, //
        { node_id : 1, node_group : 0, was_restarted : false }, //
        { node_id : 3, node_group : 1, was_restarted : false }, //
        { node_id : 5, node_group : 2, was_restarted : false }, //
        { node_id : 7, node_group : 3, was_restarted : false }, //
        { node_id : 9, node_group : 4, was_restarted : false }, //
        { node_id : 13, node_group : 6, was_restarted : false }, //
        { node_id : 15, node_group : 7, was_restarted : false }, //
        { node_id : 17, node_group : 8, was_restarted : false }, //
        { node_id : 19, node_group : 9, was_restarted : false }, //
        { node_id : 21, node_group : 10, was_restarted : false }, //
        { node_id : 23, node_group : 11, was_restarted : false }, //
        { node_id : 25, node_group : 12, was_restarted : false }, //
        { node_id : 27, node_group : 13, was_restarted : false }, //
        { node_id : 29, node_group : 14, was_restarted : false }, //
        { node_id : 31, node_group : 15, was_restarted : false }, //
        { node_id : 33, node_group : 16, was_restarted : false }, //
        { node_id : 35, node_group : 17, was_restarted : false }, //
        { node_id : 37, node_group : 18, was_restarted : false }, //
        { node_id : 39, node_group : 19, was_restarted : false }, //
        { node_id : 41, node_group : 20, was_restarted : false }, //
        { node_id : 43, node_group : 21, was_restarted : false }, //
        { node_id : 45, node_group : 22, was_restarted : false }, //
        { node_id : 47, node_group : 23, was_restarted : false } //
    };
    return test_node_sorting(nodes, 48, expected_nodes, verbose);
}

int test_node_sorting_6(int verbose)
{
    struct restart_node_status_s nodes[] = {
        //
        { node_id : 2, node_group : 0, was_restarted : false }, //
        { node_id : 3, node_group : 0, was_restarted : false }, //
        { node_id : 4, node_group : 1, was_restarted : false }, //
        { node_id : 5, node_group : 1, was_restarted : false }, //
        { node_id : 6, node_group : 2, was_restarted : false }, //
        { node_id : 7, node_group : 2, was_restarted : false } //
    };

    struct restart_node_status_s expected_nodes[] = {
        //
        { node_id : 2, node_group : 0, was_restarted : false }, //
        { node_id : 4, node_group : 1, was_restarted : false }, //
        { node_id : 6, node_group : 2, was_restarted : false }, //
        { node_id : 3, node_group : 0, was_restarted : false }, //
        { node_id : 5, node_group : 1, was_restarted : false }, //
        { node_id : 7, node_group : 2, was_restarted : false } //
    };
    return test_node_sorting(nodes, 48, expected_nodes, verbose);
}

int main(int argc, char** argv)
{
    int verbose = argc > 1 ? atoi(argv[1]) : 0;

    int failures = 0;

    failures += test_node_sorting_6(verbose);
    failures += test_node_sorting_48(verbose);

    return check_status(failures);
}
