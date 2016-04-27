TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS += -std=c99

SOURCES += main.c \
    mapped_file.c \
    chunk_manager/chunk_manager.c \
    dc_list/dc_list.c \
    hash_table/hash_funcs.c \
    hash_table/hash_table.c \
    logger/logger.c

HEADERS += \
    mapped_file.h \
    chunk_manager/chunk_manager.h \
    dc_list/dc_list.h \
    hash_table/hash_funcs.h \
    hash_table/hash_table.h \
    logger/logger.h

