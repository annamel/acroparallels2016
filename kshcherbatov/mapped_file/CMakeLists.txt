cmake_minimum_required(VERSION 2.8)
project(mappedfile)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DCMAKE_BUILD_TYPE=Release -O3 -Ofast -DNDEBUG")

set(SOURCE_FILES mapped_file.cpp logger.cpp hash_table.cpp mapped_file.cpp chunk_manager.cpp chunk_manager.h)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY out)
add_library(mappedfile ${SOURCE_FILES})