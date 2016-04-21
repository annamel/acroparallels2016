#ifdef LOG_ON

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <string.h>
#include <execinfo.h>
#include <assert.h>

#include "log.h"

#define BACKTRACE_BUF_SIZE (512)
#define BACKTRACE_ADDR_NUM  (10)

#define MSG_SIZE  (256)

struct log_msg {
	size_t size;
	char buf[MSG_SIZE];
};

static char BACKTRACE_BUF[BACKTRACE_BUF_SIZE] = {0};
static int log_level = LOG_DEFAULT;
static int logfile_fd = STDERR_FILENO;
static int instance = 0;

#define handle_log_error(msg) \
    do {perror(msg); abort();} while (0)

void log_init() {
	if(!instance)
		instance = 1;
}

void log_fini() {
	if(instance) {
		instance = 0;
		if( logfile_fd != STDERR_FILENO && close(logfile_fd) == -1 )
			handle_log_error("LOG ERROR: close");
	}
}

int log_configure(const char * logfile, int lvl) {
	if( logfile != NULL) {
		if( logfile_fd != STDERR_FILENO && close(logfile_fd) == -1 )
			handle_log_error("LOG ERROR: close");

		logfile_fd = open(logfile, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, 0666);

		if(logfile_fd == -1)
			handle_log_error("LOG ERROR: open");
	}

	if(lvl >= LOG_SUMMARY) log_level = lvl;

	return 0;
}

static size_t write_prefix(int lvl, char * dest, int fd) {
	char *prefix = NULL;

	switch(lvl) {
		case LOG_SUMMARY:
			prefix = "------------------------------------------------\nSUMMARY: ";
			break;
		case LOG_FATAL:
			prefix = (fd != STDERR_FILENO && fd != STDOUT_FILENO) ? "FATAL " : "\x1b[31m[FATAL]\x1b[0m ";
			break;
		case LOG_ERR:
			prefix = (fd != STDERR_FILENO && fd != STDOUT_FILENO) ? "ERROR: " : "\x1b[31m[ERROR]\x1b[0m ";
			break;
		case LOG_WARN:
			prefix = (fd != STDERR_FILENO && fd != STDOUT_FILENO) ? "WARNING: " : "\x1b[33m[WARN]\x1b[0m  ";
			break;
		case LOG_INFO:
			prefix = (fd != STDERR_FILENO && fd != STDOUT_FILENO) ? "INFO: " : "\x1b[32m[INFO]\x1b[0m  ";
			break;
		case LOG_DEBUG:
			prefix = (fd != STDERR_FILENO && fd != STDOUT_FILENO) ? "DEBUG" : "\x1b[35m[DEBUG]\x1b[0m ";
			break;
		default:
			return 0;
			break;
	}

	strcpy(dest, prefix);
	return strlen(prefix);
}

static struct log_msg msg = {0, {0}};

int log_write(int lvl, const char *fmt, ...) {
	if(fmt == NULL)
		return EINVAL;

	if(lvl > log_level)
		return 0;

	msg.size = write_prefix(lvl, msg.buf, logfile_fd);

	va_list va;
	va_start(va, fmt);
	int bytenum = vsnprintf(msg.buf + msg.size, MSG_SIZE, fmt, va);
	if(bytenum < 0)
		handle_log_error("LOG ERROR: vsnprintf");
	va_end(va);

	msg.size += bytenum;

	if( write(logfile_fd, msg.buf, msg.size) == -1 )
		handle_log_error("LOG ERROR: write");

	if( lvl == LOG_FATAL ) {
		int addrnum = backtrace( (void**)BACKTRACE_BUF, BACKTRACE_ADDR_NUM );

		backtrace_symbols_fd( (void**)BACKTRACE_BUF, addrnum, logfile_fd );
	}

	return 0;
}

#endif