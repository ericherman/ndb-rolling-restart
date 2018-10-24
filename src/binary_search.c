/*
 * binary_search.c
 * Copyright (C) 2018 Chris Veenboer <git@interdictor.org> and
 * Eric Herman <eric@freesa.org>
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

#include "binary_search.h"
#include <assert.h>

enum binary_search_result binary_search_ints(int* elements, size_t num_elements,
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

    size_t span = num_elements;
    size_t mid = num_elements / 2;
    size_t large_half;
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
