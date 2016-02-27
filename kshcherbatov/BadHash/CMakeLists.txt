cmake_minimum_required(VERSION 3.4)
project(HashTable)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp hash_table.cpp vm_service.cpp vm_service.h)
add_executable(HashTable ${SOURCE_FILES})