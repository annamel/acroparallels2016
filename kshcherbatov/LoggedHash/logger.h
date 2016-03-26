//
// Created by kir on 12.03.16.
//

#ifndef DLOG_H
#define DLOG_H

enum LOGLEVEL_TYPES {
    LOGLEVEL_UNUSED,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG,
    LOGLEVEL_WARN,
    LOGLEVEL_ERROR,
    LOGLEVEL_FATAL
};

#ifndef DEBUG
    #define DEBUG LOGLEVEL_UNUSED
#endif

#ifdef DEBUG
    #include <unistd.h>

    #ifndef DEFAULT_LOG_FILENAME
        #define DEFAULT_LOG_FILENAME "log.txt"
    #endif

    struct ring_buff_t;
    extern struct ring_buff_t *Logging_system;

    void ring_buf_push_msg(struct ring_buff_t *ring_buff, enum LOGLEVEL_TYPES log_level, pid_t pid, const char *fmt, ...);
    void init_log_system(const char log_filename[]);
    void deinit_log_system(void);

    #if DEBUG<=LOGLEVEL_FATAL
    #define LOG_FATAL(fmt, args...) ring_buf_push_msg(Logging_system, LOGLEVEL_FATAL, getpid(), fmt, ##args)
    #else
    #define LOG_FATAL(fmt, args...)
    #endif

    #if DEBUG<=LOGLEVEL_ERROR
    #define LOG_ERROR(fmt, args...) ring_buf_push_msg(Logging_system, LOGLEVEL_ERROR, getpid(), fmt, ##args)
    #else
    #define LOG_ERROR(fmt, args...)
    #endif

    #if DEBUG<=LOGLEVEL_WARN
    #define LOG_WARN(fmt, args...) ring_buf_push_msg(Logging_system, LOGLEVEL_WARN, getpid(), fmt, ##args)
    #else
    #define LOG_WARN(fmt, args...)
    #endif

    #if DEBUG<=LOGLEVEL_DEBUG
    #define LOG_DEBUG(fmt, args...) ring_buf_push_msg(Logging_system, LOGLEVEL_DEBUG, getpid(), fmt, ##args)
    #else
    #define LOG_DEBUG(fmt, args...)
    #endif

    #if DEBUG<=LOGLEVEL_INFO
    #define LOG_INFO(fmt, args...) ring_buf_push_msg(Logging_system, LOGLEVEL_INFO, getpid(), fmt, ##args)
    #else
    #define LOG_INFO(fmt, args...)
    #endif
#else
    #define LOG_FATAL(fmt, args...)
    #define LOG_ERROR(fmt, args...)
    #define LOG_WARN(fmt, args...)
    #define LOG_DEBUG(fmt, args...)
    #define LOG_INFO(fmt, args...)
#endif

#endif //DLOG_H
