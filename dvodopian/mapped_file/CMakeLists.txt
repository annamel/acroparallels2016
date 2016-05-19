cmake_minimum_required(VERSION 3.5)
project(file_mapper_project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")


add_subdirectory(file_mapper)
#add_subdirectory(api_adapter)
add_subdirectory(key_set)
add_subdirectory(sorted_list)
add_subdirectory(chunk_manager)
add_subdirectory(mapped_file)
add_subdirectory(tests)

include_directories(file_mapper api_adapter)


add_executable(file_mapper_project main.c)

#target_include_directories(file_mapper_project PRIVATE file_mapper key_set)
#target_link_libraries(file_mapper_project file_mapper key_set)

#add_library(mappedfile STATIC ${SOURCES})


#install(TARGETS mapped_file EXPORT mappedfile DESTINATION out)

#--------------
#add_library(mappedfile STATIC mapped_file/mapped_file.c mapped_file/mapped_file.h)
#target_include_directories(mappedfile "key_set" "sorted_list" "chunk_manager")
#
#target_link_libraries(mappedfile key_set sorted_list chunk_manager)