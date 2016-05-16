#ifndef LOGGER_H
#define LOGGER_H

#include "ret_code.h"

#define LOG_MAX_MSG_LEN         256
#define LOG_MAX_MSG_PREFIX_LEN  32
#define LOG_MAX_BACKTRACE_LEN   256

#define LOG_DUMP_LOAD 0.8 // Must be from 0 to 1

typedef enum
        {
        LOG_DEBUG   = 0,
        LOG_INFO    = 1,
        LOG_WARNING = 2,
        LOG_ERROR   = 3,
        LOG_FATAL   = 4,
        } log_level_t;


typedef struct logger   logger_t;
typedef struct log_line log_line_t;

ret_code_t log_init  (log_level_t log_filter_level, const char * log_file_path);
ret_code_t log_deinit();

ret_code_t log_write(log_level_t log_level, char * format, ...);


#endif // LOGGER_H
