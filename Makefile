# Makefile - ndb-rolling-restart
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
COMPILER := $(shell readlink c++)

ifeq ($(UNAME), Darwin)
	DYNAMIC_LIB_EXT=dylib
else
	DYNAMIC_LIB_EXT=so
endif


CXX_INCLUDES=-I$(MYSQL_NDB_DIR)/include/storage/ndb


CXX=/usr/bin/c++
FORMAT=clang-format-6.0 --style=WebKit


NDB_LDFLAGS_DYNAMIC=\
	-fsanitize=address \
	-rdynamic \
	-L$(MYSQL_NDB_DIR)/lib \
	-Wl,-rpath,$(MYSQL_NDB_DIR)/lib
NDB_LIBS_DYNAMIC=$(MYSQL_NDB_DIR)/lib/libndbclient.$(DYNAMIC_LIB_EXT)
NDB_LD_ADD_DYNAMIC=-lndbclient


ifeq ($(COMPILER), gcc)
	NDB_LDFLAGS_STATIC=-static-libasan
endif
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

LDFLAGS=$(NDB_LDFLAGS)

LDADD=$(NDB_LD_ADD)

ndb-rolling-restart: ndb-rolling-restart.cpp
	$(CXX) -c $(CXXFLAGS) ndb-rolling-restart.cpp -o ndb-rolling-restart.o
	$(CXX) $(LDFLAGS) ndb-rolling-restart.o $(NDB_LIBS) -o ndb-rolling-restart $(LDADD)

tidy:
	$(FORMAT) ndb-rolling-restart.cpp > ndb-rolling-restart.cpp.format
	cp ndb-rolling-restart.cpp ndb-rolling-restart.cpp~
	mv ndb-rolling-restart.cpp.format ndb-rolling-restart.cpp

clean:
	rm -vf *.o ndb-rolling-restart
