//
// Created by kir on 12.03.16.
//

#ifndef DLOG_H
#define DLOG_H

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define LOG_MSG_MAX_SIZE 1024
#define LOG_FILENAME "log.txt"

#define DEBUG LOGLEVEL_UNUSED

enum LOGLEVEL_TYPES {
    LOGLEVEL_UNUSED,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG,
    LOGLEVEL_WARN,
    LOGLEVEL_ERROR,
    LOGLEVEL_FATAL
};


#ifdef DEBUG
    struct ring_buff_t;
    extern struct ring_buff_t *Logging_system;

    int ring_buff_push_msg(struct ring_buff_t *ring_buff, const char msg[]);

    #define CALL_LOG_FUNC(err_log_lvl, fmt, args...)\
    do {\
        char msg[LOG_MSG_MAX_SIZE];\
        snprintf(msg, LOG_MSG_MAX_SIZE, fmt, ## args);\
        ring_buff_push_msg(Logging_system, msg);\
    } while (0)

    #if DEBUG<=LOGLEVEL_FATAL
    #define LOG_FATAL(fmt, args...) CALL_LOG_FUNC(LOGLEVEL_ERROR, fmt, ##args)
    #else
    #define LOG_FATAL(fmt, args...)
    #endif

    #if DEBUG<=LOGLEVEL_ERROR
    #define LOG_ERROR(fmt, args...) CALL_LOG_FUNC(LOGLEVEL_ERROR, fmt, ##args)
    #else
    #define LOG_ERROR(fmt, args...)
    #endif

    #if DEBUG<=LOGLEVEL_WARN
    #define LOG_WARN(fmt, args...) CALL_LOG_FUNC(LOGLEVEL_ERROR, fmt, ##args)
    #else
    #define LOG_WARN(fmt, args...)
    #endif

    #if DEBUG<=LOGLEVEL_DEBUG
    #define LOG_DEBUG(fmt, args...) CALL_LOG_FUNC(LOGLEVEL_ERROR, fmt, ##args)
    #else
    #define LOG_DEBUG(fmt, args...)
    #endif

    #if DEBUG<=LOGLEVEL_INFO
    #define LOG_INFO(fmt, args...) CALL_LOG_FUNC(LOGLEVEL_ERROR, fmt, ##args)
    #else
    #define LOG_INFO(fmt, args...)
    #endif

    void __init_log_system(int argc, char * argv[]) __attribute__ ((constructor));
    void __deinit_log_system(void) __attribute__ ((destructor));
#else
    #define LOG_FATAL(fmt, args...)
    #define LOG_ERROR(fmt, args...)
    #define LOG_WARN(fmt, args...)
    #define LOG_DEBUG(fmt, args...)
    #define LOG_INFO(fmt, args...)
#endif

#endif //DLOG_H
