//
// Created by voddan on 15/03/16.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"

#ifndef MELEHOVA_TRAININGS_LOGLIB_H
#define MELEHOVA_TRAININGS_LOGLIB_H

typedef enum {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    _print_log_level
} LogLevel;

#define DEFAULT_BUF_SIZE (256)
#define CALLSTACK_MAX_SIZE (10)

void log_init(FILE * stream, size_t size, LogLevel level);

void log_write(LogLevel level, char const * msg);

void log_flush();

void log_set_level(LogLevel level);

//--------------------

#define logger(level) log_init(fopen("./log.txt", "w+"), DEFAULT_BUF_SIZE, level)

#define flogger(path, level) log_init(fopen(path, "w+"), DEFAULT_BUF_SIZE, level)

#define logerr(level) log_init(stderr, DEFAULT_BUF_SIZE, level)
#define logout(level) log_init(stdout, DEFAULT_BUF_SIZE, level)

#define debug(msg) log_write(DEBUG, msg)
#define info(msg) log_write( INFO, msg)
#define warn(msg) log_write( WARN, msg)
#define error(msg) log_write(ERROR, msg)

#endif //MELEHOVA_TRAININGS_LOGLIB_H

#pragma clang diagnostic pop