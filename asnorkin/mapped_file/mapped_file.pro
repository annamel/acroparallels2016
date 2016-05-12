TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS += -std=c99

SOURCES += main.c \
    chunk_manager/chunk_manager.c \
    mapped_file.c \
    dc_list/dc_list.c \
    hash_table/hash_funcs.c \
    hash_table/hash_table.c \
    logger/logger.c \
    test/asnorkin_test.c \
    sorted_set/sorted_set.c \
    test/test_stairs_vac.c

HEADERS += \
    chunk_manager/chunk_manager.h \
    asnorkin_test.h \
    mapped_file.h \
    dc_list/dc_list.h \
    hash_table/hash_funcs.h \
    hash_table/hash_table.h \
    logger/logger.h \
    sorted_set/sorted_set.h

