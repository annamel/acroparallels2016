cmake_minimum_required(VERSION 2.8)
project(mappedfile)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -DCMAKE_BUILD_TYPE=Release -Ofast -DNDEBUG")

set(SOURCE_FILES mapped_file.cpp logger.cpp hash_table.cpp)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY out)
add_library(mappedfile ${SOURCE_FILES})
