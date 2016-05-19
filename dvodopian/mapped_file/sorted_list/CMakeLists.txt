cmake_minimum_required(VERSION 3.5)
project(sorted_list)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")


set(HEADER_FILES sorted_list.h)

set(SOURCE_FILES sorted_list.c)


add_library(sorted_list STATIC ${SOURCE_FILES} ${HEADER_FILES})
