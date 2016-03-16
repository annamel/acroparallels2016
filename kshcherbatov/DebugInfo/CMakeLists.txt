cmake_minimum_required(VERSION 2.8)
project(DebugInfo)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp logger.cpp logger.h)
add_executable(DebugInfo ${SOURCE_FILES})