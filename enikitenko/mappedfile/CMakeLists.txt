project (Logger)
cmake_minimum_required (VERSION 2.8)

set (WITH_LOGGER                  OFF  CACHE BOOL   "Enable logger")

list (APPEND SOURCES
	mapped_file.c
	hashtable.c)

set (CMAKE_C_FLAGS "-Ofast")
set (CMAKE_SHARED_LINKER_FLAGS "-Ofast")

if (WITH_LOGGER)
	list (APPEND SOURCES
		../logger/log.c
		../logger/linux.c
		../logger/not_demangle.c)
	add_definitions (-DWITH_LOGGER)
endif (WITH_LOGGER)

add_library (mappedfile ${SOURCES})

if (WITH_LOGGER)
	target_link_libraries (testapp dl pthread)
endif (WITH_LOGGER)
