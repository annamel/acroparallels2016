cmake_minimum_required(VERSION 3.5)
project(par_file_mapper)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

add_subdirectory(mapped_file)

if(EXISTS tests)
    add_subdirectory(tests)
endif()