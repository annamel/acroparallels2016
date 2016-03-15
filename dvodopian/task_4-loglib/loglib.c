//
// Created by voddan on 15/03/16.
//

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "loglib.h"

typedef struct {
    char * buf;
    size_t buf_size;
    char * eob;
    LogLevel level;
} Logger;


Logger * Logger_new(size_t size, LogLevel level) {
    Logger * log = (Logger *) malloc(sizeof(Logger));

    log->buf_size = size;
    log->buf = (char *) calloc(sizeof(char), log->buf_size);
    log->eob = log->buf;
    log->level = level;

    return log;
}

/** input `Logger` on start, input `NULL` to clear, any other input has no effect */
Logger * _Logger_singleton_set(Logger * log) {
    static Logger * instance = NULL;

    if (!instance || !log) {
        instance = log;
    }

    return instance;
}

Logger * Logger_singleton() {
    return _Logger_singleton_set((Logger *) true);
}

void Logger_destruct() {
    Logger * instance = Logger_singleton();

    free(instance->buf);
    free(instance);

    _Logger_singleton_set(NULL);
}

//---- PUBLIC API -----------------

void log_init(size_t size, LogLevel level) {
    Logger * log = Logger_new(size, level);
    _Logger_singleton_set(log);
    atexit(Logger_destruct);
}

void log_write(LogLevel level, char const * msg) {
    Logger * log = Logger_singleton();

    if (log->level > level)
        return;

    char * middle = log->buf + log->buf_size / 2;

    if (log->eob > middle)
        log_flush();

    size_t size_left = log->buf + log->buf_size - log->eob;

// todo: better format
    snprintf(log->eob, size_left, ">>> %s\n", msg);
}

void log_flush() {
    Logger * log = Logger_singleton();

//    todo: write to file here

    log->eob = log->buf;
}

void log_set_level(LogLevel level) {
    Logger * log = Logger_singleton();
    log->level = level;
}
