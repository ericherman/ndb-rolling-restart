cmake_minimum_required (VERSION 3.1)
project (ndb-rolling-restart)

SET (NDBINCLUDE "/usr/include/mysql" CACHE PATH "Include Path")
SET (NDBLIB "/usr/lib64/mysql/" CACHE PATH "Library path")

find_package (Threads REQUIRED)

set (CMAKE_CXX_FLAGS "-fsanitize=address -g -Wall --std=c++11")

include_directories (${NDBINCLUDE}/storage/ndb)
link_directories (${NDBLIB})

add_executable (ndb_rolling_restart ndb-rolling-restart.cpp)
target_link_libraries (ndb_rolling_restart ndbclient)

install (TARGETS ndb_rolling_restart DESTINATION bin)
