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
    size
} LogLevel;

#define DEFAULT_LOG_LEVEL (LogLevel.INFO)
#define DEFAULT_BUF_SIZE (256)

void log_init(size_t size, LogLevel level);

void log_write(LogLevel level, char const * msg);

void log_flush();

void log_set_level(LogLevel level);

#endif //MELEHOVA_TRAININGS_LOGLIB_H

#pragma clang diagnostic pop