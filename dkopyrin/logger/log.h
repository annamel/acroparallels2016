#ifndef __LOG_H
#define __LOG_H
#define DEBUG LOGLEVEL_NOT_USED

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

enum log_level {
	LOGLEVEL_NOT_USED, //This level is used for unused or flushed events
	LOGLEVEL_INFO,
	LOGLEVEL_DEBUG,
	LOGLEVEL_WARN,
	LOGLEVEL_ERROR,
	LOGLEVEL_FATAL
};
struct ring_buf;
struct ring_buf *rb;

int ring_buf_add_event(struct ring_buf *rb, enum log_level level, pid_t pid, const char *fmt, ...);

#ifdef DEBUG
#if DEBUG<=LOGLEVEL_FATAL
#define LOG_FATAL(fmt, args...) ring_buf_add_event(rb, LOGLEVEL_FATAL, getpid(), fmt, ##args)
#else
#define LOG_FATAL(fmt, args...)
#endif

#if DEBUG<=LOGLEVEL_ERROR
#define LOG_ERROR(fmt, args...) ring_buf_add_event(rb, LOGLEVEL_ERROR, getpid(), fmt, ##args)
#else
#define LOG_ERROR(fmt, args...)
#endif

#if DEBUG<=LOGLEVEL_WARN
#define LOG_WARN(fmt, args...) ring_buf_add_event(rb, LOGLEVEL_WARN, getpid(), fmt, ##args)
#else
#define LOG_WARN(fmt, args...)
#endif

#if DEBUG<=LOGLEVEL_DEBUG
#define LOG_DEBUG(fmt, args...) ring_buf_add_event(rb, LOGLEVEL_DEBUG, getpid(), fmt, ##args)
#else
#define LOG_DEBUG(fmt, args...)
#endif

#if DEBUG<=LOGLEVEL_INFO
#define LOG_INFO(fmt, args...) ring_buf_add_event(rb, LOGLEVEL_INFO, getpid(), fmt, ##args)
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

//This calls can be done from main if attributes are not supported
void log_start (int argc, char *argv[]) __attribute__ ((constructor));
void log_clean (void) __attribute__ ((destructor));

#endif
