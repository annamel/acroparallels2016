cmake_minimum_required(VERSION 3.5)
project(test_file_mapper_project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

add_subdirectory(lib/googletest-release-1.7.0)
include_directories(lib/googletest-release-1.7.0/include)

add_executable(             test-internal-api test-internal-api.cpp)
target_link_libraries(      test-internal-api gtest gtest_main)
target_link_libraries(      test-internal-api file_mapper)
target_include_directories( test-internal-api PRIVATE ../file_mapper/file_mapper.h)

add_executable(             test-api-adapter test-api-adapter.cpp)
target_link_libraries(      test-api-adapter gtest gtest_main)
target_link_libraries(      test-api-adapter api_adapter)
target_include_directories( test-api-adapter PRIVATE ../api_adapter/api_adapter.h)