#ifndef LOGGER_H
#define LOGGER_H

/*  
 *	The main advantage of logger is providing  
 *	log messages with unfixed length for better 
 *  description of errors. 
 *  In the case of fatal error logger flushes all possible 
 *  log messages into log file and prints fatal error messages
 *  into the same file.  
 *  Written by Semyon Abramov
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <execinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef DEBUG_MODE

#define BUFFER_SIZE 4096
#define CALL_TRACE_BUF_SIZE	250 
#define LOG_FILE "log_file"

typedef enum log_level 
{
	FATAL_LEVEL,
	ERROR_LEVEL,
	WARNING_LEVEL, 
	DEBUG_LEVEL,
	INFO_LEVEL
} log_level_t;

struct Logger 
{
	log_level_t level;
	FILE* log_file;
};	

char* conf_log_file;

char* call_trace_buf [CALL_TRACE_BUF_SIZE];
char* write_p;
int filedesc;
char log_buffer[BUFFER_SIZE];
struct Logger* logger; 

void flush_buf();

#define log_string(str, message, nbytes, ...) \
	do { \
		nbytes = snprintf(write_p, (uint8_t*)log_buffer + \
		BUFFER_SIZE - (uint8_t*)write_p, str message "\n", ##__VA_ARGS__); \
	} while(0)	

#define log_with_level(log_lev, nbytes, message,  ...) \
	do { \
 		switch (log_lev) \
 		{ \
 			case ERROR_LEVEL: \
 				log_string("error: ", message, nbytes, ##__VA_ARGS__); \
 				break; \
 			case WARNING_LEVEL: \
				log_string("warning: ", message, nbytes, ##__VA_ARGS__); \
				break;\
			case DEBUG_LEVEL: \
				log_string("debug: ",  message, nbytes, ##__VA_ARGS__);  \
				break;\
			case INFO_LEVEL: \
				log_string("info: ", message, nbytes, ##__VA_ARGS__);  \
				break;\
		} \
	} while(0)

#define log_message(log_lev, message, ...) \
	do { \
		if (logger == NULL) \
		{	\
			logger = malloc(sizeof(struct Logger)); \
			write_p = log_buffer; \
			if (conf_log_file) \
				filedesc = open(conf_log_file, O_RDWR | O_CREAT, S_IRWXU); \
			else \
				filedesc = open(LOG_FILE, O_RDWR | O_CREAT, S_IRWXU); \
			logger->log_file = fdopen(filedesc, "r+"); \
			logger->level = INFO_LEVEL; \
			atexit(flush_buf); \
		} \
		if (logger->level >= log_lev)  \
		{ \
			if (log_lev == FATAL_LEVEL) \
			{ \
				fprintf(logger->log_file, "%s\n", message); \
				fprintf(logger->log_file, "%s\n%s\n%s\n", "Application failed with FATAL ERROR.", \
				"Call trace is represented above.", "Other saved logs available below in order from the earliest to the latest log"); \
				size_t btrace_size = backtrace((void**)call_trace_buf, CALL_TRACE_BUF_SIZE); \
				backtrace_symbols_fd((void**)call_trace_buf, btrace_size, filedesc); \
				flush_buf(logger);  \
			}  \
			else \
			{ \
				int nbytes; \
				log_with_level(log_lev, nbytes, message, ##__VA_ARGS__); \
				if (nbytes >= (uint8_t*)log_buffer + BUFFER_SIZE - (uint8_t*)write_p) \
				{	\
							stall_cur_buf(); \
							log_with_level(log_lev, nbytes, message, ##__VA_ARGS__); \
							write_p = (char*)log_buffer; \
				} \
				else \
				{  \
					write_p = (uint8_t*)(write_p) + nbytes; \
				} \
			}	\
		}	\
	} while(0)

#else  // DEBUG_MODE

#define log_string(...) do { } while(0) 
#define log_with_level(...) do { } while(0)
#define log_message(...) do { } while(0)
#define log_fatal() do { } while(0)

#endif 


#endif // LOGGER_H

			
				  
