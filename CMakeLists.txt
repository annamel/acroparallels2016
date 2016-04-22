cmake_minimum_required(VERSION 3.3)
project(acroparallels2016)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    aivanov/badhash/hash.c
    aneganov/hellhash/hell.c
    aneganov/hellhash/hellhash.h
    aneganov/hellhash/main.c
    azhadchenko/tester.c
    dkopyrin/hash/dehash.h
    dkopyrin/hash/hash.c
    dkopyrin/hash/test.c
    dokhlopkov/hash/hash.c
    dokhlopkov/hash/hash.h
    dokhlopkov/hash/main.c
    dvodopian/brain_fuck_machine.c
    dvodopian/brain_fuck_machine.h
    dvodopian/hashfunction_bfm.c
    ikrotov/hashfunc.c
    ikrotov/stdgrib.h
    ikrotov/stdl1b.h
    ikrotov/stdlo.h
    kiskakov/awful_hash/awful_hash.c
    kiskakov/awful_hash/mainfuncs.h
    kiskakov/awful_hash/nums.h
    kshcherbatov/BadHash/hash_table.cpp
    kshcherbatov/BadHash/hash_table.h
    kshcherbatov/BadHash/main.cpp
    kshcherbatov/BadHash/vm_service.cpp
    kshcherbatov/BadHash/vm_service.h
    sabramov/Badhash/arl.h
    sabramov/Badhash/hash.c
    sabramov/Badhash/hash.h
    README.md)

add_executable(acroparallels2016 ${SOURCE_FILES})