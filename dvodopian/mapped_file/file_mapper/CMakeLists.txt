cmake_minimum_required(VERSION 3.5)
project(file_mapper)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")




set(HEADER_FILES file_mapper.h)

set(SOURCE_FILES file_mapper.c)


add_library(file_mapper STATIC ${SOURCE_FILES} ${HEADER_FILES})

include_directories("../key_set" "../sorted_list")
target_link_libraries(file_mapper key_set sorted_list)
