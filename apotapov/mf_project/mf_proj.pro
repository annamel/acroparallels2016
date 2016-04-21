TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    chunkManager.c \
    hash.c \
    mappedfile.c \
    logger.c

HEADERS += \
    chunkManager.h \
    hash.h \
    mappedfile.h \
    logger.h

