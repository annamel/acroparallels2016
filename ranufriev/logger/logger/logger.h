#ifndef __LOGGER_H_INCLUDED
#define __LOGGER_H_INCLUDED


#include <stdarg.h>
// 'logger' supports thread-safe implementation of 'my_errno' (TLS)
#include "../error/error.h"


#define IS_LOGGER_ENABLED 1

// positive integer in range of [0..100) indicating maximum occupancy percentage
#define MAX_BUF_OCCUPANCY 75

#define MAX_LENGTH_OF_LOG_STRING 1024
#define MAX_LENGTH_OF_BACKTRACE  64

typedef enum LogLevels
        {
        LOG_DEBUG = 0,
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR,
        LOG_FATAL,

        NUMB_OF_LOG_LEVELS
        } log_level_t;

// 'Logger' is singleton; 'construct' and 'destruct' is not thread safe!
int Logger_construct (log_level_t min_log_level, const char* log_file_path);
int Logger_destruct  ();

// use this func similar to 'printf'
int Logger_log (log_level_t log_level, const char* format_str, ...);


#endif // __LOGGER_H_INCLUDED
