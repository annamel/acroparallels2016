cmake_minimum_required(VERSION 3.5)
project(file_mapper_project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")


add_subdirectory(file_mapper)
add_subdirectory(tests)
add_subdirectory(api_adapter)

#include_directories(file_mapper api_adapter)


add_executable(file_mapper_project main.c)
target_link_libraries(file_mapper_project file_mapper)