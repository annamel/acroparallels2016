#ifdef LOG_ON

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <execinfo.h>

#include "log.h"

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define BACKTRACE_BUF_SIZE (512)
#define BACKTRACE_ADDR_NUM  (1024)

#define MSG_SIZE  (1024)

struct log_msg {
	size_t size;
	char buf[MSG_SIZE];
};

static char BACKTRACE_BUF[BACKTRACE_BUF_SIZE] = {0};
static int log_level = LOG_DEBUG;
static int logfile_fd;

void log_init(const char * logfile, int lvl) {
	log_level = lvl;

	logfile_fd = open(logfile, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666);

	if(logfile_fd == -1)
		handle_error("open");
}

void log_fini() {
	if( close(logfile_fd) == -1 )
		handle_error("close");
}

static size_t write_prefix(int lvl, char * dest) {
	switch(lvl) {
		case LOG_FATAL:
			strcpy(dest, "FATAL: ");
			return strlen("FATAL: ");
			break;
		case LOG_ERR:
			strcpy(dest, "ERROR: ");
			return strlen("ERROR: ");
			break;
		case LOG_WARN:
			strcpy(dest, "WARNING: ");
			return strlen("WARNING: ");
			break;
		case LOG_INFO:
			strcpy(dest, "INFO: ");
			return strlen("INFO: ");
			break;
		case LOG_DEBUG:
			strcpy(dest, "DEBUG: ");
			return strlen("DEBUG: ");
			break;
		default:
			return 0;
			break;
	}
}

static struct log_msg msg = {0, {0}};

void log_write(int lvl, const char *fmt, ...) {
	if(lvl > log_level)
		return;

	msg.size = write_prefix(lvl, msg.buf);

	va_list va;
	va_start(va, fmt);
	int bytenum = vsnprintf(msg.buf + msg.size, MSG_SIZE, fmt, va);
	if(bytenum < 0)
		handle_error("vsnprintf");
	va_end(va);

	msg.size += bytenum;

	if( write(logfile_fd, msg.buf, msg.size) == -1 )
		handle_error("write");

	if( lvl == LOG_FATAL ) {
		int addrnum = backtrace( (void**)BACKTRACE_BUF, BACKTRACE_ADDR_NUM );

		backtrace_symbols_fd( (void**)BACKTRACE_BUF, addrnum, logfile_fd );
	}
}

#endif
