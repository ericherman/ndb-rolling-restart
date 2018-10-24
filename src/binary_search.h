/*
 * binary_search.h
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

#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> /* size_t */

enum binary_search_result {
    binary_search_error = 0,
    binary_search_found = 1,
    binary_search_insert = 2
};

/**
 * Search elements for a target
 * returns
 *    binary_search_found: target is an element of elements,
 *	target_index is set to the location
 *    binary_search_insert: target is NOT an element of elements,
 *      target_index is set to the index where one would need to insert target
 *    binary_search_error: elements is NULL, target_index is unchanged
 */
enum binary_search_result binary_search_ints(int* elements, size_t num_elements,
    int target, size_t* target_index);

#ifdef __cplusplus
}
#endif

#endif /* ifndef BINARY_SEARCH_H */
