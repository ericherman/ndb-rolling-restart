#include <stdlib.h>

#include "echeck.h"
#include "ndb_rolling_restart.hpp"
#include <string.h>

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

void init_node_status(struct restart_node_status_s *node, int id, int group)
{
	node->node_id = id;
	node->node_group = group;
	node->was_restarted = false;
}

int test_node_sorting_48(int verbose)
{
    struct restart_node_status_s nodes[48];
    struct restart_node_status_s expected_nodes[48];

    size_t i = 0;

    memset(nodes, 0, sizeof(struct restart_node_status_s) * 48);
    memset(expected_nodes, 0, sizeof(struct restart_node_status_s) * 48);

    init_node_status(&(nodes[i++]), 47, 23);
    init_node_status(&(nodes[i++]), 20, 9);
    init_node_status(&(nodes[i++]), 39, 19);
    init_node_status(&(nodes[i++]), 26, 12);
    init_node_status(&(nodes[i++]), 6, 2);
    init_node_status(&(nodes[i++]), 37, 18);
    init_node_status(&(nodes[i++]), 7, 3);
    init_node_status(&(nodes[i++]), 29, 14);
    init_node_status(&(nodes[i++]), 40, 19);
    init_node_status(&(nodes[i++]), 25, 12);
    init_node_status(&(nodes[i++]), 36, 17);
    init_node_status(&(nodes[i++]), 1, 0);
    init_node_status(&(nodes[i++]), 8, 3);
    init_node_status(&(nodes[i++]), 9, 4);
    init_node_status(&(nodes[i++]), 30, 14);
    init_node_status(&(nodes[i++]), 14, 6);
    init_node_status(&(nodes[i++]), 31, 15);
    init_node_status(&(nodes[i++]), 13, 6);
    init_node_status(&(nodes[i++]), 21, 10);
    init_node_status(&(nodes[i++]), 16, 7);
    init_node_status(&(nodes[i++]), 42, 20);
    init_node_status(&(nodes[i++]), 3, 1);
    init_node_status(&(nodes[i++]), 24, 11);
    init_node_status(&(nodes[i++]), 5, 2);
    init_node_status(&(nodes[i++]), 2, 0);
    init_node_status(&(nodes[i++]), 18, 8);
    init_node_status(&(nodes[i++]), 11, 5);
    init_node_status(&(nodes[i++]), 27, 13);
    init_node_status(&(nodes[i++]), 33, 16);
    init_node_status(&(nodes[i++]), 44, 21);
    init_node_status(&(nodes[i++]), 38, 18);
    init_node_status(&(nodes[i++]), 15, 7);
    init_node_status(&(nodes[i++]), 10, 4);
    init_node_status(&(nodes[i++]), 32, 15);
    init_node_status(&(nodes[i++]), 4, 1);
    init_node_status(&(nodes[i++]), 46, 22);
    init_node_status(&(nodes[i++]), 48, 23);
    init_node_status(&(nodes[i++]), 43, 21);
    init_node_status(&(nodes[i++]), 28, 13);
    init_node_status(&(nodes[i++]), 41, 20);
    init_node_status(&(nodes[i++]), 35, 17);
    init_node_status(&(nodes[i++]), 22, 10);
    init_node_status(&(nodes[i++]), 12, 5);
    init_node_status(&(nodes[i++]), 19, 9);
    init_node_status(&(nodes[i++]), 34, 16);
    init_node_status(&(nodes[i++]), 45, 22);
    init_node_status(&(nodes[i++]), 17, 8);
    init_node_status(&(nodes[i++]), 23, 11);

    i = 0;
    init_node_status(&(expected_nodes[i++]), 1, 0);
    init_node_status(&(expected_nodes[i++]), 3, 1);
    init_node_status(&(expected_nodes[i++]), 5, 2);
    init_node_status(&(expected_nodes[i++]), 7, 3);
    init_node_status(&(expected_nodes[i++]), 9, 4);
    init_node_status(&(expected_nodes[i++]), 13, 6);
    init_node_status(&(expected_nodes[i++]), 15, 7);
    init_node_status(&(expected_nodes[i++]), 17, 8);
    init_node_status(&(expected_nodes[i++]), 19, 9);
    init_node_status(&(expected_nodes[i++]), 21, 10);
    init_node_status(&(expected_nodes[i++]), 23, 11);
    init_node_status(&(expected_nodes[i++]), 25, 12);
    init_node_status(&(expected_nodes[i++]), 27, 13);
    init_node_status(&(expected_nodes[i++]), 29, 14);
    init_node_status(&(expected_nodes[i++]), 31, 15);
    init_node_status(&(expected_nodes[i++]), 33, 16);
    init_node_status(&(expected_nodes[i++]), 35, 17);
    init_node_status(&(expected_nodes[i++]), 37, 18);
    init_node_status(&(expected_nodes[i++]), 39, 19);
    init_node_status(&(expected_nodes[i++]), 41, 20);
    init_node_status(&(expected_nodes[i++]), 43, 21);
    init_node_status(&(expected_nodes[i++]), 45, 22);
    init_node_status(&(expected_nodes[i++]), 47, 23);
    init_node_status(&(expected_nodes[i++]), 2, 0);
    init_node_status(&(expected_nodes[i++]), 4, 1);
    init_node_status(&(expected_nodes[i++]), 6, 2);
    init_node_status(&(expected_nodes[i++]), 8, 3);
    init_node_status(&(expected_nodes[i++]), 10, 4);
    init_node_status(&(expected_nodes[i++]), 12, 5);
    init_node_status(&(expected_nodes[i++]), 14, 6);
    init_node_status(&(expected_nodes[i++]), 16, 7);
    init_node_status(&(expected_nodes[i++]), 18, 8);
    init_node_status(&(expected_nodes[i++]), 20, 9);
    init_node_status(&(expected_nodes[i++]), 22, 10);
    init_node_status(&(expected_nodes[i++]), 24, 11);
    init_node_status(&(expected_nodes[i++]), 26, 12);
    init_node_status(&(expected_nodes[i++]), 28, 13);
    init_node_status(&(expected_nodes[i++]), 30, 14);
    init_node_status(&(expected_nodes[i++]), 32, 15);
    init_node_status(&(expected_nodes[i++]), 34, 16);
    init_node_status(&(expected_nodes[i++]), 36, 17);
    init_node_status(&(expected_nodes[i++]), 38, 18);
    init_node_status(&(expected_nodes[i++]), 40, 19);
    init_node_status(&(expected_nodes[i++]), 42, 20);
    init_node_status(&(expected_nodes[i++]), 44, 21);
    init_node_status(&(expected_nodes[i++]), 46, 22);
    init_node_status(&(expected_nodes[i++]), 48, 23);

    return test_node_sorting(nodes, 48, expected_nodes, verbose);
}

int test_node_sorting_6(int verbose)
{
    struct restart_node_status_s nodes[6];
    struct restart_node_status_s expected_nodes[6];

    memset(nodes, 0, sizeof(struct restart_node_status_s) * 6);
    memset(expected_nodes, 0, sizeof(struct restart_node_status_s) * 6);

    init_node_status(&(nodes[0]), 2, 0);
    init_node_status(&(nodes[1]), 3, 0);
    init_node_status(&(nodes[2]), 4, 1);
    init_node_status(&(nodes[3]), 5, 1);
    init_node_status(&(nodes[4]), 6, 2);
    init_node_status(&(nodes[5]), 7, 2);

    init_node_status(&(expected_nodes[0]), 2, 0);
    init_node_status(&(expected_nodes[1]), 4, 1);
    init_node_status(&(expected_nodes[2]), 6, 2);
    init_node_status(&(expected_nodes[3]), 3, 0);
    init_node_status(&(expected_nodes[4]), 5, 1);
    init_node_status(&(expected_nodes[5]), 7, 2);

    return test_node_sorting(nodes, 6, expected_nodes, verbose);
}

int main(int argc, char** argv)
{
    int verbose = argc > 1 ? atoi(argv[1]) : 0;

    int failures = 0;

    failures += test_node_sorting_6(verbose);
    failures += test_node_sorting_48(verbose);

    return check_status(failures);
}
