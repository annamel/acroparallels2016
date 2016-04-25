cmake_minimum_required(VERSION 2.8)
project(mappedfile)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
if(MSVC)
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-variadic-macros -ansi -pedantic -D__STDC_FORMAT_MACROS -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -D_GNU_SOURCE -D_REENTRANT -Wall -Wextra -Wwrite-strings -Winit-self -Wcast-align -Wcast-qual -Wpointer-arith -Wstrict-aliasing -Wformat=2 -Wuninitialized -Wmissing-declarations -Woverloaded-virtual -Wnon-virtual-dtor -Wctor-dtor-privacy -Wno-long-long -DCMAKE_BUILD_TYPE=Release")
endif()

set(SOURCE_FILES mapped_file.cpp logger.cpp hash_table.cpp mapped_file.cpp chunk_manager.cpp chunk_manager.h)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY out)
add_library(mappedfile ${SOURCE_FILES})