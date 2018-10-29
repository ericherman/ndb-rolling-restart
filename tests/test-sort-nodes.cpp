#include <stdlib.h>

#include "echeck.h"
#include "ndb_rolling_restart.hpp"
#include <string.h>

int test_node_sorting(std::vector<restart_node_status_s>& nodes,
    const std::vector<restart_node_status_s>& expected_nodes, int verbose)
{
    int failures = check_int_m(nodes.size(), expected_nodes.size(),
        "node sizes should match");
    if (failures) {
        return 1;
    }

    if (verbose) {
        printf("Nodes in input order:\n");
        size_t i = 0;
        for (const auto& node : nodes) {
            printf("Node[%02lu]: node_id:%d, group = %d\n", (unsigned long)i,
                node.node_id, node.node_group);
            ++i;
        }
    }

    sort_node_restarts(nodes);

    if (verbose) {
        printf("Nodes in output order:\n");
        size_t i = 0;
        for (const auto& node : nodes) {
            printf("Node[%02lu]: node_id:%d, group = %d\n", (unsigned long)i,
                node.node_id, node.node_group);
            ++i;
        }
    }

    for (size_t i = 0; i < nodes.size(); ++i) {
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
    std::vector<restart_node_status_s> nodes = {
        restart_node_status_s{ 47, 23, false },
        restart_node_status_s{ 20, 9, false },
        restart_node_status_s{ 39, 19, false },
        restart_node_status_s{ 26, 12, false },
        restart_node_status_s{ 6, 2, false },
        restart_node_status_s{ 37, 18, false },
        restart_node_status_s{ 7, 3, false },
        restart_node_status_s{ 29, 14, false },
        restart_node_status_s{ 40, 19, false },
        restart_node_status_s{ 25, 12, false },
        restart_node_status_s{ 36, 17, false },
        restart_node_status_s{ 1, 0, false },
        restart_node_status_s{ 8, 3, false },
        restart_node_status_s{ 9, 4, false },
        restart_node_status_s{ 30, 14, false },
        restart_node_status_s{ 14, 6, false },
        restart_node_status_s{ 31, 15, false },
        restart_node_status_s{ 13, 6, false },
        restart_node_status_s{ 21, 10, false },
        restart_node_status_s{ 16, 7, false },
        restart_node_status_s{ 42, 20, false },
        restart_node_status_s{ 3, 1, false },
        restart_node_status_s{ 24, 11, false },
        restart_node_status_s{ 5, 2, false },
        restart_node_status_s{ 2, 0, false },
        restart_node_status_s{ 18, 8, false },
        restart_node_status_s{ 11, 5, false },
        restart_node_status_s{ 27, 13, false },
        restart_node_status_s{ 33, 16, false },
        restart_node_status_s{ 44, 21, false },
        restart_node_status_s{ 38, 18, false },
        restart_node_status_s{ 15, 7, false },
        restart_node_status_s{ 10, 4, false },
        restart_node_status_s{ 32, 15, false },
        restart_node_status_s{ 4, 1, false },
        restart_node_status_s{ 46, 22, false },
        restart_node_status_s{ 48, 23, false },
        restart_node_status_s{ 43, 21, false },
        restart_node_status_s{ 28, 13, false },
        restart_node_status_s{ 41, 20, false },
        restart_node_status_s{ 35, 17, false },
        restart_node_status_s{ 22, 10, false },
        restart_node_status_s{ 12, 5, false },
        restart_node_status_s{ 19, 9, false },
        restart_node_status_s{ 34, 16, false },
        restart_node_status_s{ 45, 22, false },
        restart_node_status_s{ 17, 8, false },
        restart_node_status_s{ 23, 11, false }
    };

    std::vector<restart_node_status_s> expected_nodes = {
        restart_node_status_s{ 2, 0, false },
        restart_node_status_s{ 4, 1, false },
        restart_node_status_s{ 6, 2, false },
        restart_node_status_s{ 8, 3, false },
        restart_node_status_s{ 10, 4, false },
        restart_node_status_s{ 12, 5, false },
        restart_node_status_s{ 14, 6, false },
        restart_node_status_s{ 16, 7, false },
        restart_node_status_s{ 18, 8, false },
        restart_node_status_s{ 20, 9, false },
        restart_node_status_s{ 22, 10, false },
        restart_node_status_s{ 24, 11, false },
        restart_node_status_s{ 26, 12, false },
        restart_node_status_s{ 28, 13, false },
        restart_node_status_s{ 30, 14, false },
        restart_node_status_s{ 32, 15, false },
        restart_node_status_s{ 34, 16, false },
        restart_node_status_s{ 36, 17, false },
        restart_node_status_s{ 38, 18, false },
        restart_node_status_s{ 40, 19, false },
        restart_node_status_s{ 42, 20, false },
        restart_node_status_s{ 44, 21, false },
        restart_node_status_s{ 46, 22, false },
        restart_node_status_s{ 48, 23, false },
        restart_node_status_s{ 1, 0, false },
        restart_node_status_s{ 3, 1, false },
        restart_node_status_s{ 5, 2, false },
        restart_node_status_s{ 7, 3, false },
        restart_node_status_s{ 9, 4, false },
        restart_node_status_s{ 11, 5, false },
        restart_node_status_s{ 13, 6, false },
        restart_node_status_s{ 15, 7, false },
        restart_node_status_s{ 17, 8, false },
        restart_node_status_s{ 19, 9, false },
        restart_node_status_s{ 21, 10, false },
        restart_node_status_s{ 23, 11, false },
        restart_node_status_s{ 25, 12, false },
        restart_node_status_s{ 27, 13, false },
        restart_node_status_s{ 29, 14, false },
        restart_node_status_s{ 31, 15, false },
        restart_node_status_s{ 33, 16, false },
        restart_node_status_s{ 35, 17, false },
        restart_node_status_s{ 37, 18, false },
        restart_node_status_s{ 39, 19, false },
        restart_node_status_s{ 41, 20, false },
        restart_node_status_s{ 43, 21, false },
        restart_node_status_s{ 45, 22, false },
        restart_node_status_s{ 47, 23, false }
    };

    return test_node_sorting(nodes, expected_nodes, verbose);
}

int test_node_sorting_6(int verbose)
{
    std::vector<restart_node_status_s> nodes = {
        restart_node_status_s{ 2, 0, false },
        restart_node_status_s{ 3, 0, false },
        restart_node_status_s{ 4, 1, false },
        restart_node_status_s{ 5, 1, false },
        restart_node_status_s{ 6, 2, false },
        restart_node_status_s{ 7, 2, false }
    };

    std::vector<restart_node_status_s> expected_nodes = {
        restart_node_status_s{ 3, 0, false },
        restart_node_status_s{ 5, 1, false },
        restart_node_status_s{ 7, 2, false },
        restart_node_status_s{ 2, 0, false },
        restart_node_status_s{ 4, 1, false },
        restart_node_status_s{ 6, 2, false }
    };

    return test_node_sorting(nodes, expected_nodes, verbose);
}

int test_node_sorting_16_by_3(int verbose)
{
    std::vector<restart_node_status_s> nodes = {
        restart_node_status_s{ 2, 0, false },
        restart_node_status_s{ 3, 0, false },
        restart_node_status_s{ 4, 0, false },
        restart_node_status_s{ 5, 1, false },
        restart_node_status_s{ 6, 1, false },
        restart_node_status_s{ 7, 1, false },
        restart_node_status_s{ 8, 2, false },
        restart_node_status_s{ 9, 2, false },
        restart_node_status_s{ 10, 2, false },
        restart_node_status_s{ 11, 3, false },
        restart_node_status_s{ 12, 3, false },
        restart_node_status_s{ 13, 3, false },
        restart_node_status_s{ 14, 4, false },
        restart_node_status_s{ 15, 4, false },
        restart_node_status_s{ 16, 4, false },
        restart_node_status_s{ 17, 5, false },
        restart_node_status_s{ 18, 5, false },
        restart_node_status_s{ 19, 5, false },
        restart_node_status_s{ 20, 6, false },
        restart_node_status_s{ 21, 6, false },
        restart_node_status_s{ 22, 6, false },
        restart_node_status_s{ 23, 7, false },
        restart_node_status_s{ 24, 7, false },
        restart_node_status_s{ 25, 7, false },
        restart_node_status_s{ 26, 8, false },
        restart_node_status_s{ 27, 8, false },
        restart_node_status_s{ 28, 8, false },
        restart_node_status_s{ 29, 9, false },
        restart_node_status_s{ 30, 9, false },
        restart_node_status_s{ 31, 9, false },
        restart_node_status_s{ 32, 10, false },
        restart_node_status_s{ 33, 10, false },
        restart_node_status_s{ 34, 10, false },
        restart_node_status_s{ 35, 11, false },
        restart_node_status_s{ 36, 11, false },
        restart_node_status_s{ 37, 11, false },
        restart_node_status_s{ 38, 12, false },
        restart_node_status_s{ 39, 12, false },
        restart_node_status_s{ 40, 12, false },
        restart_node_status_s{ 41, 13, false },
        restart_node_status_s{ 42, 13, false },
        restart_node_status_s{ 43, 13, false },
        restart_node_status_s{ 44, 14, false },
        restart_node_status_s{ 45, 14, false },
        restart_node_status_s{ 46, 14, false },
        restart_node_status_s{ 47, 15, false },
        restart_node_status_s{ 48, 15, false },
        restart_node_status_s{ 49, 15, false }
    };

    std::vector<restart_node_status_s> expected_nodes = {
        restart_node_status_s{ 4, 0, false },
        restart_node_status_s{ 7, 1, false },
        restart_node_status_s{ 10, 2, false },
        restart_node_status_s{ 13, 3, false },
        restart_node_status_s{ 16, 4, false },
        restart_node_status_s{ 19, 5, false },
        restart_node_status_s{ 22, 6, false },
        restart_node_status_s{ 25, 7, false },
        restart_node_status_s{ 28, 8, false },
        restart_node_status_s{ 31, 9, false },
        restart_node_status_s{ 34, 10, false },
        restart_node_status_s{ 37, 11, false },
        restart_node_status_s{ 40, 12, false },
        restart_node_status_s{ 43, 13, false },
        restart_node_status_s{ 46, 14, false },
        restart_node_status_s{ 49, 15, false },

        restart_node_status_s{ 3, 0, false },
        restart_node_status_s{ 6, 1, false },
        restart_node_status_s{ 9, 2, false },
        restart_node_status_s{ 12, 3, false },
        restart_node_status_s{ 15, 4, false },
        restart_node_status_s{ 18, 5, false },
        restart_node_status_s{ 21, 6, false },
        restart_node_status_s{ 24, 7, false },
        restart_node_status_s{ 27, 8, false },
        restart_node_status_s{ 30, 9, false },
        restart_node_status_s{ 33, 10, false },
        restart_node_status_s{ 36, 11, false },
        restart_node_status_s{ 39, 12, false },
        restart_node_status_s{ 42, 13, false },
        restart_node_status_s{ 45, 14, false },
        restart_node_status_s{ 48, 15, false },

        restart_node_status_s{ 2, 0, false },
        restart_node_status_s{ 5, 1, false },
        restart_node_status_s{ 8, 2, false },
        restart_node_status_s{ 11, 3, false },
        restart_node_status_s{ 14, 4, false },
        restart_node_status_s{ 17, 5, false },
        restart_node_status_s{ 20, 6, false },
        restart_node_status_s{ 23, 7, false },
        restart_node_status_s{ 26, 8, false },
        restart_node_status_s{ 29, 9, false },
        restart_node_status_s{ 32, 10, false },
        restart_node_status_s{ 35, 11, false },
        restart_node_status_s{ 38, 12, false },
        restart_node_status_s{ 41, 13, false },
        restart_node_status_s{ 44, 14, false },
        restart_node_status_s{ 47, 15, false }
    };

    return test_node_sorting(nodes, expected_nodes, verbose);
}

int main(int argc, char** argv)
{
    int verbose = argc > 1 ? atoi(argv[1]) : 0;

    int failures = 0;

    failures += test_node_sorting_6(verbose);
    failures += test_node_sorting_48(verbose);
    failures += test_node_sorting_16_by_3(verbose);

    return check_status(failures);
}
