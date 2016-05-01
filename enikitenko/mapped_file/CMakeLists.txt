project (Logger)
cmake_minimum_required (VERSION 2.8)

set (WITH_LOGGER                  OFF  CACHE BOOL   "Enable logger")
set (DEBUG_MODE                   OFF  CACHE BOOL   "Debug mode")

list (APPEND SOURCES
	mapped_file.c
	hashtable.c)

set (CMAKE_C_FLAGS "-Ofast")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY out)

if (WITH_LOGGER)
	list (APPEND SOURCES
		../logger/log.c
		../logger/linux.c
		../logger/not_demangle.c)
	add_definitions (-DWITH_LOGGER)
endif (WITH_LOGGER)

if (DEBUG_MODE)
	add_definitions (-DDEBUG_MODE)
endif (DEBUG_MODE)

add_library (mappedfile ${SOURCES})
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../include)

if (WITH_LOGGER)
	target_link_libraries (testapp dl pthread)
endif (WITH_LOGGER)
