cmake_minimum_required(VERSION 3.3)
project(mf)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c99")

list (APPEND SOURCES
        sources/mallocs.c
        sources/chunks.c
        sources/mapped_file.c
        sources/hashtable/hashfunction.c
        sources/hashtable/hashtable.c
        sources/logger/logger.c)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)

#for linkage by global tester
add_library(mappedfile STATIC ${SOURCES})

add_executable(tester test/test_simple.c)
#add_executable(tester test/test_performance.c)
target_link_libraries(tester mappedfile)