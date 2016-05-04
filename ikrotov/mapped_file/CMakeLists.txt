cmake_minimum_required(VERSION 3.2)
project(mf)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -std=c99")

list (APPEND SOURCES ChunkManager.c mf_lib/mapped_file.c List/list.c hash_table/hash_table.c hash_table/hash_funcs.c)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)

add_library(mappedfile STATIC ${SOURCES})
add_executable(tester tester/test.c)
target_link_libraries(tester mappedfile)
