project (mapped_file)
cmake_minimum_required (VERSION 2.8)

set (WITH_LOGGER                  OFF  CACHE BOOL   "Enable logger")
set (DEBUG_MODE                   OFF  CACHE BOOL   "Enable debug mode")
set (MULTITHREADING               OFF  CACHE BOOL   "Enable multithreading support")

if (DEBUG_MODE)
	set (MMAP_PROTECTION          ON   CACHE BOOL   "Enable mmap protection")
endif (DEBUG_MODE)

list (APPEND SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/mapped_file.c
	${CMAKE_CURRENT_SOURCE_DIR}/hashtable.c)

set (CMAKE_C_FLAGS "-Ofast")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY out)

if (WITH_LOGGER)
	list (APPEND SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/../logger/log.c
		${CMAKE_CURRENT_SOURCE_DIR}/../logger/linux.c
		${CMAKE_CURRENT_SOURCE_DIR}/../logger/not_demangle.c)
	add_definitions (-DWITH_LOGGER)
endif (WITH_LOGGER)

if (MULTITHREADING)
	add_definitions (-DMULTITHREADING)
endif (MULTITHREADING)

if (DEBUG_MODE)
	add_definitions (-DDEBUG_MODE)

	if (MMAP_PROTECTION)
		add_definitions (-DMMAP_PROTECTION)
	endif (MMAP_PROTECTION)
endif (DEBUG_MODE)

add_library (mappedfile ${SOURCES})
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../include)

if (WITH_LOGGER)
	target_link_libraries (testapp dl pthread)
endif (WITH_LOGGER)
