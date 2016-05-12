cmake_minimum_required(VERSION 2.8)
project (mapped_file)

set (CMAKE_C_FLAGS "-DNDEBUG -Ofast")
set (CMAKE_CXX_FLAGS "-DNDEBUG -Ofast")
set (CMAKE_SHARED_LINKER_FLAGS "-DNDEBUG -Ofast")
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "out")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "out")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Werror")

add_subdirectory(log)

#for linkage by global tester
add_library(mappedfile STATIC FileRegion.cpp MappedFile.cpp mapped_file.cpp log/log.c log/logger.c)
target_link_libraries(mappedfile pthread rt)

add_executable(test1 test/test1.c log)
target_link_libraries(test1 mappedfile pthread rt)

add_executable(test2 test/test2.c log)
target_link_libraries(test2 mappedfile pthread rt)

