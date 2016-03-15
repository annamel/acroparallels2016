project (Logger)
cmake_minimum_required (VERSION 2.8)

list (APPEND SHARED_SOURCES
	log.c
	linux.c)

list (APPEND CLOG_SOURCES
	test.c
	not_demangle.c)

list (APPEND CXXLOG_SOURCES
	test.cpp
	g++_demangle.cpp)

list (APPEND CMAKE_EXE_LINKER_FLAGS -rdynamic)

set (CMAKE_SHARED_LINKER_FLAGS -rdynamic)

add_executable (clog ${SHARED_SOURCES} ${CLOG_SOURCES})
add_executable (cxxlog ${SHARED_SOURCES} ${CXXLOG_SOURCES})

target_link_libraries (clog dl pthread)
target_link_libraries (cxxlog dl pthread)
