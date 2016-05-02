#ifndef __LOG_H
#define __LOG_H

#include "log_types.h"
#include "log_ring_buf.h"

enum log_level cur_log_level;
struct ring_buf log_ring_buf;

#ifdef DEBUG_LOG
#define INFO  LOGLEVEL_INFO
#define DEBUG LOGLEVEL_DEBUG
#define WARN  LOGLEVEL_WARN
#define ERROR LOGLEVEL_ERROR
#define FATAL LOGLEVEL_FATAL

#ifndef LOGCOLOR
#define LOGCOLOR(x) x
#endif

#define LOG(loglevel, fmt, args...) if (loglevel > cur_log_level) { 			\
	ring_buf_add_event(&log_ring_buf, loglevel, getpid(), LOGCOLOR(fmt), ##args);	\
}
#else
#define LOG(loglevel, fmt, args...)
#endif

#endif
