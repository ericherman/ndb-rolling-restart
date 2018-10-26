# Makefile - ndb_rolling_restart
# Copyright (C) 2018 Eric Herman <eric@freesa.org>
#
# This work is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This work is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

MYSQL_NDB_DIR=$(HOME)/builds/mysql-cluster-7.6
NDB_CXX_STD=--std=c++11

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	DYNAMIC_LIB_EXT=dylib
else
	DYNAMIC_LIB_EXT=so
endif

CXX_INCLUDES=-I$(MYSQL_NDB_DIR)/include/storage/ndb


SHELL := /bin/bash
CXX=/usr/bin/c++
CC=/usr/bin/cc
FORMAT=clang-format-6.0 --style=WebKit

ifeq ($(WITH_ASAN), 1)
	ASAN=-static-libasan -fsanitize=address
endif

NDB_LDFLAGS_DYNAMIC=\
	$(ASAN) \
	-rdynamic \
	-L$(MYSQL_NDB_DIR)/lib \
	-Wl,-rpath,$(MYSQL_NDB_DIR)/lib
NDB_LIBS_DYNAMIC=$(MYSQL_NDB_DIR)/lib/libndbclient.$(DYNAMIC_LIB_EXT)
NDB_LD_ADD_DYNAMIC=-lndbclient


NDB_LDFLAGS_STATIC=-static-libasan
NDB_LIBS_STATIC=\
	$(MYSQL_NDB_DIR)/lib/libndbclient_static.a \
	$(MYSQL_NDB_DIR)/lib/libmysqlclient.a
NDB_LD_ADD_STATIC=-pthread


ifeq "$(BUILD_TYPE)" "static"
NDB_LDFLAGS=$(NDB_LDFLAGS_STATIC)
NDB_LIBS=$(NDB_LIBS_STATIC)
NDB_LD_ADD=$(NDB_LD_ADD_STATIC)
else
NDB_LDFLAGS=$(NDB_LDFLAGS_DYNAMIC)
NDB_LIBS=$(NDB_LIBS_DYNAMIC)
NDB_LD_ADD=$(NDB_LD_ADD_DYNAMIC)
endif

CXX_NDB_SRC_WORKAROUNDS=-Wno-unused-parameter

ifneq "$(DEBUG)" "1"
DEBUG_CFLAGS=-DNDBUG
endif

CXXFLAGS=$(NDB_CXX_STD) \
  -g -O2 -Wall -Wextra -Werror \
  $(DEBUG_CFLAGS) \
  $(CXX_NDB_SRC_WORKAROUNDS) \
  $(CXX_INCLUDES)

CFLAGS=-std=c11 \
  -g -O2 -Wall -Wextra -Werror \
  $(DEBUG_CFLAGS)

LDFLAGS=$(NDB_LDFLAGS)

LDADD=$(NDB_LD_ADD)

ndb_rolling_restart: ndb_rolling_restart.o \
		src/ndb_rolling_restart.hpp src/ndb_rolling_restart_main.cpp
	$(CXX) -c $(CXXFLAGS) src/ndb_rolling_restart_main.cpp \
		-o ndb_rolling_restart_main.o
	$(CXX) $(LDFLAGS) \
		ndb_rolling_restart.o \
		ndb_rolling_restart_main.o \
		$(NDB_LIBS) \
		-o ndb_rolling_restart $(LDADD)

ndb_rolling_restart.o: src/ndb_rolling_restart.hpp \
		src/ndb_rolling_restart.cpp
	$(CXX) -c $(CXXFLAGS) src/ndb_rolling_restart.cpp \
		-o ndb_rolling_restart.o

echeck.o: tests/echeck.h tests/echeck.c
	$(CC) -c $(CFLAGS) -Itests/ tests/echeck.c -o echeck.o

test-sort-nodes: echeck.o ndb_rolling_restart \
		tests/test-sort-nodes.cpp
	$(CXX) $(CXXFLAGS) -Itests/ -Isrc/ \
		tests/test-sort-nodes.cpp \
		$(LDFLAGS) \
		ndb_rolling_restart.o \
		echeck.o \
		$(NDB_LIBS) \
		-o test-sort-nodes $(LDADD)

check-sort-nodes: test-sort-nodes
	./test-sort-nodes

check: check-sort-nodes

tidy:
	for FILE in \
		`find src tests -name '*.h' -o -name '*.c' | grep -v echeck` \
 		`find src tests -name '*.hpp' -o -name '*.cpp'` \
		; do $(FORMAT) $$FILE > $${FILE}.format; \
		cp $${FILE} $${FILE}~; \
                mv $${FILE}.format $${FILE}; \
        done

clean:
	rm -vf *.o ndb_rolling_restart \
		test-binary-search-int-basic \
		test-sort-nodes
