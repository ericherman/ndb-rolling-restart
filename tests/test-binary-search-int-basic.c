/*
test-binary-search-basic.c
Copyright (C) 2018 Eric Herman <eric@freesa.org>

This work is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This work is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

	https://www.gnu.org/licenses/lgpl-3.0.txt
	https://www.gnu.org/licenses/gpl-3.0.txt
*/
#include <stdio.h>
#include <string.h>

#include "../src/binary_search.h"
#include "echeck.h"

int test_binary_search_int_found(int verbose)
{
    size_t elements_len = 10;
    int elements[10];

    int failures = 0;

    size_t num_elements = 5;
    for (size_t i = 0; i < elements_len; ++i) {
        if (i < num_elements) {
            elements[i] = 1 + (i * 2);
        } else {
            elements[i] = 0;
        }
    }

    size_t expected_target_index = 3;
    int target = elements[expected_target_index];
    size_t target_index = 99;
    enum binary_search_result expected_search_result = binary_search_found;

    if (verbose) {
        for (size_t i = 0; i < elements_len; ++i) {
            printf("elements[%lu]: %d\n", (unsigned long)i,
                elements[i]);
        }
        printf("searching for %d\n", target);
        printf("expect at: %lu\n", expected_target_index);
    }
    enum binary_search_result search_result = binary_search_ints(elements,
        num_elements, target, &target_index);

    failures += check_int((int)search_result, expected_search_result);
    failures += check_size_t(target_index, expected_target_index);

    return failures;
}

int main(int argc, char* argv[])
{
    int verbose;
    int failures = 0;

    verbose = (argc > 1) ? atoi(argv[1]) : 0;

    failures += test_binary_search_int_found(verbose);

    if (failures) {
        fprintf(stderr, "%d failures in %s\n", failures, __FILE__);
    }
    return check_status(failures);
}
