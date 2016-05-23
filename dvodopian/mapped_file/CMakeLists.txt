cmake_minimum_required(VERSION 3.5)
project(file_mapper_project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

add_subdirectory(mapperonio)

if(EXISTS tests)
    add_subdirectory(tests)
endif()

#add_subdirectory(file_mapper)
#add_subdirectory(api_adapter)
#add_subdirectory(key_set)
#add_subdirectory(sorted_list)
#add_subdirectory(chunk_manager)
#add_subdirectory(mapped_file)