cmake_minimum_required(VERSION 3.5)
project(api_adapter)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")


set(HEADER_FILES api_adapter.h)

set(SOURCE_FILES api_adapter.c)


add_library(api_adapter STATIC ${SOURCE_FILES} ${HEADER_FILES})
