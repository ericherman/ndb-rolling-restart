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
#include <assert.h>
#include <getopt.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

#define Cerr cerr << __FILE__ << ":" << __LINE__ << ": "

/* Global */
int verbose_flag = 0;

/* Global */
static struct option long_options[] = {
    { "connection_string", required_argument, NULL, 'c' },
    { "wait_seconds", required_argument, NULL, 'w' },
    { "verbose", no_argument, &verbose_flag, 1 },
    { 0, 0, 0, 0 }
};

int main(int argc, char** argv)
{
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

    return ndb_rolling_restart(&ndb_ctx);
}
