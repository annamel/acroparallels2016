TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS += -std=c99

SOURCES += main.c \
    hash_table/hash_table.c \
    logger/logger.c \
    chunk_manager/chunk_manager.c \
    mapped_file.c

HEADERS += \
    mapped_file.h \
    hash_table/hash_table.h \
    logger/logger.h \
    chunk_manager/chunk_manager.h
