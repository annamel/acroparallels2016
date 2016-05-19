cmake_minimum_required(VERSION 3.5)
project(chunk_manager)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")




set(HEADER_FILES chunk_manager.h)

set(SOURCE_FILES chunk_manager.c)


add_library(chunk_manager STATIC ${SOURCE_FILES} ${HEADER_FILES})

include_directories("../key_set" "../sorted_list")
target_link_libraries(chunk_manager key_set sorted_list)
