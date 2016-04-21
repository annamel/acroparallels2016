cmake_minimum_required(VERSION 3.3)
project(mf)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES sources/mallocs.c sources/chunks.c sources/mapped_file.c)
set(HASHTABLE sources/hashtable/hashtable.c sources/hashtable/hashfunction.c sources/logger/logger.c)
add_executable(mf ${HASHTABLE} ${SOURCE_FILES} tests/test_mf.c)