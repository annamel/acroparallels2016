cmake_minimum_required(VERSION 3.3)
project(hashtable)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(SOURCE_FILES hashtable.c hashfunction.c)
add_executable(hashtable ${SOURCE_FILES} test_hashtable.c)