cmake_minimum_required(VERSION 3.5)
project(mappedfile)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")


set(HEADER_FILES mapped_file.h)
set(SOURCE_FILES mapped_file.c)


add_library(mappedfile STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(mappedfile pthread)
set_target_properties(mappedfile PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out)
