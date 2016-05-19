cmake_minimum_required(VERSION 3.5)
project(key_set)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")


set(HEADER_FILES key_set.h)

set(SOURCE_FILES ordered_list.c)


add_library(key_set STATIC ${SOURCE_FILES} ${HEADER_FILES})
