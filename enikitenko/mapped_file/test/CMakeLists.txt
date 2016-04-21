project (mappedfile_test)
cmake_minimum_required (VERSION 2.8)

list (APPEND SOURCES
	test.c)

set (CMAKE_C_FLAGS "-Ofast")
set (CMAKE_SHARED_LINKER_FLAGS "-Ofast")

add_executable (testapp ${SOURCES})
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../include)
target_link_libraries(testapp ${CMAKE_CURRENT_SOURCE_DIR}/../out/libmappedfile.a)
