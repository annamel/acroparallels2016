cmake_minimum_required(VERSION 2.8)
project (mapped_file)

set(LOG OFF)
set(REGION_PROTECTION OFF)
set(MAPPED_FILE_MT ON)

if (LOG)
	add_definitions (-DLOG)
endif (LOG)

if (REGION_PROTECTION)
	add_definitions (-DREGION_PROTECTION)
endif (REGION_PROTECTION)

if (MAPPED_FILE_MT)
	add_definitions (-DMAPPED_FILE_MT)
endif (MAPPED_FILE_MT)

set(CMAKE_C_FLAGS "-DNDEBUG -Ofast")
set(CMAKE_CXX_FLAGS "-DNDEBUG -Ofast")
set(CMAKE_SHARED_LINKER_FLAGS "-DNDEBUG -Ofast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Werror")

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "out")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "out")


add_library(mappedfile STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/FileRegion.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MappedFile.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mapped_file.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/log/log.c
	${CMAKE_CURRENT_SOURCE_DIR}/log/logger.c)
	
target_link_libraries(mappedfile pthread rt)
