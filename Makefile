#
# static build is not working :-(
#

MYSQL_NDB_DIR=/home/eric/builds/mysql-cluster-7.6
NDB_CXX_STD=--std=c++11

CXX_INCLUDES=-I$(MYSQL_NDB_DIR)/include/storage/ndb


CXX=/usr/bin/c++
FORMAT=clang-format-6.0 --style=WebKit


NDB_LDFLAGS_DYNAMIC=-fsanitize=address \
        -rdynamic \
	$(MYSQL_NDB_DIR)/lib/libndbclient.so \
	-L$(MYSQL_NDB_DIR)/lib \
	-Wl,-rpath,$(MYSQL_NDB_DIR)/lib
NDB_LD_ADD_DYNAMIC=-lndbclient


NDB_LDFLAGS_STATIC=-static \
	$(MYSQL_NDB_DIR)/lib/libndbclient_static.a
NDB_LD_ADD_STATIC=


NDB_LDFLAGS=$(NDB_LDFLAGS_DYNAMIC)
NDB_LD_ADD=$(NDB_LD_ADD_DYNAMIC)
# NDB_LDFLAGS=$(NDB_LDFLAGS_STATIC)
# NDB_LD_ADD=$(NDB_LD_ADD_STATIC)

CXX_NDB_SRC_WORKAROUNDS=-Wno-unused-parameter

CXXFLAGS=$(NDB_CXX_STD) \
  -g -O2 -Wall -Wextra -Werror \
  $(CXX_NDB_SRC_WORKAROUNDS) \
  $(CXX_INCLUDES)

LDFLAGS=$(NDB_LDFLAGS)

LDADD=$(NDB_LD_ADD)

ndb-rolling-restart: ndb-rolling-restart.cpp
	$(CXX) -c $(CXXFLAGS) ndb-rolling-restart.cpp -o ndb-rolling-restart.o
	$(CXX) $(LDFLAGS) ndb-rolling-restart.o -o ndb-rolling-restart $(LDADD)

tidy:
	$(FORMAT) ndb-rolling-restart.cpp > ndb-rolling-restart.cpp.format
	cp ndb-rolling-restart.cpp ndb-rolling-restart.cpp~
	mv ndb-rolling-restart.cpp.format ndb-rolling-restart.cpp

clean:
	rm -vf *.o ndb-rolling-restart
