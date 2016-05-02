#ifndef __LOG_TYPES_H
#define __LOG_TYPES_H
#define MAX_MSG_SIZE 256

#include <unistd.h>
#include <sys/types.h>

enum log_level {
	LOGLEVEL_NOT_USED, //This level is used for unused or flushed events
	LOGLEVEL_INFO,
	LOGLEVEL_DEBUG,
	LOGLEVEL_WARN,
	LOGLEVEL_ERROR,
	LOGLEVEL_FATAL
};

struct event {
        pid_t pid;
        enum log_level level;
        char msg[MAX_MSG_SIZE];
};
#endif
