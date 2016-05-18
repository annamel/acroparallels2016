project (parallel_mapped_file)
cmake_minimum_required (VERSION 2.8)

set (MAPPED_FILE_MT ON)
set (CMAKE_CURRENT_SOURCE_DIR ../mapped_file)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
