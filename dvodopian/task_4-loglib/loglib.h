//
// Created by voddan on 15/03/16.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"

#ifndef MELEHOVA_TRAININGS_LOGLIB_H
#define MELEHOVA_TRAININGS_LOGLIB_H

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    _LOG_print_log_level
} LogLevel;

#define DEFAULT_BUF_SIZE (256)
#define CALLSTACK_MAX_SIZE (10)

void log_init(FILE * stream, size_t size, LogLevel level);

void log_write(LogLevel level, char const * msg);

void log_flush();

void log_set_level(LogLevel level);

//--------------------

#ifndef RELEASE  //RELEASE

#define logger(level) log_init(fopen("./log.txt", "w+"), DEFAULT_BUF_SIZE, level)

#define flogger(path, level) log_init(fopen(path, "w+"), DEFAULT_BUF_SIZE, level)

#define logERR(level) log_init(stderr, DEFAULT_BUF_SIZE, level)
#define logOUT(level) log_init(stdout, DEFAULT_BUF_SIZE, level)

#define debug(msg)   log_write(LOG_DEBUG, msg)
#define info(msg)    log_write( LOG_INFO, msg)
#define warning(msg) log_write( LOG_WARN, msg)
#define error(msg)   log_write(LOG_ERROR, msg)

#define loggerFlush() log_flush()

#define loggerLevel(level) log_set_level(level)

#else  //RELEASE
#define logger(level)         0
#define flogger(path, level)  0
#define logERR(level)         0
#define logOUT(level)         0
#define debug(msg)            0
#define info(msg)             0
#define warning(msg)          0
#define error(msg)            0
#define loggerFlush()         0
#define loggerLevel(level)    0
#endif  //RELEASE


#endif //MELEHOVA_TRAININGS_LOGLIB_H

#pragma clang diagnostic pop