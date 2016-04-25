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


//#ifndef DEBUG
//    #define DEBUG LOGLEVEL_UNUSED
//#endif

#ifndef DEFAULT_LOG_FILENAME
    #define DEFAULT_LOG_FILENAME "log.txt"
#endif


#ifdef DEBUG
    #include <unistd.h>

    struct ring_buff_t;
    extern struct ring_buff_t *Logging_system;

    void ring_buf_push_msg(struct ring_buff_t *ring_buff, enum LOGLEVEL_TYPES log_level,
                           const pid_t pid, const char *fmt, ...);

    void __init_log_system(int argc, char * argv[])  __attribute__ ((constructor));
    void __deinit_log_system(void) __attribute__ ((destructor));

    #if DEBUG<=LOGLEVEL_FATAL
    #define LOG_FATAL(fmt, ...) ring_buf_push_msg(Logging_system, LOGLEVEL_FATAL, getpid(), fmt, ##__VA_ARGS__)
    #else
    #define LOG_FATAL(fmt, ...)
    #endif

    #if DEBUG<=LOGLEVEL_ERROR
    #define LOG_ERROR(fmt, ...) ring_buf_push_msg(Logging_system, LOGLEVEL_ERROR, getpid(), fmt, ##__VA_ARGS__)
    #else
    #define LOG_ERROR(fmt, args...)
    #endif

    #if DEBUG<=LOGLEVEL_WARN
    #define LOG_WARN(fmt, ...) ring_buf_push_msg(Logging_system, LOGLEVEL_WARN, getpid(), fmt, ##__VA_ARGS__)
    #else
    #define LOG_WARN(fmt, ...)
    #endif

    #if DEBUG<=LOGLEVEL_DEBUG
    #define LOG_DEBUG(fmt, ...) ring_buf_push_msg(Logging_system, LOGLEVEL_DEBUG, getpid(), fmt, ##__VA_ARGS__)
    #else
    #define LOG_DEBUG(fmt, ...)
    #endif

    #if DEBUG<=LOGLEVEL_INFO
    #define LOG_INFO(fmt, ...) ring_buf_push_msg(Logging_system, LOGLEVEL_INFO, getpid(), fmt, ##__VA_ARGS__)
    #else
    #define LOG_INFO(fmt, ...)
    #endif
#else
    #define LOG_FATAL( ...)
    #define LOG_ERROR( ...)
    #define LOG_WARN( ...)
    #define LOG_DEBUG( ...)
    #define LOG_INFO(...)
#endif

#endif //DLOG_H
