#ifndef LOGGER_H
#define LOGGER_H

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

#define BUFFER_SIZE 2048
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


typedef enum assoc_of_buf
{
	TWO_WAY = 2,
	FOUR_WAY = 4,
	EIGHT_WAY = 8
} assoc_of_buf_t;

struct Logger 
{
	log_level_t level;
	FILE* log_file;
	int buf_assoc;
	int cur_buf;
	int size_of_buf;
};	

char* call_trace_buf [CALL_TRACE_BUF_SIZE];
char** log_buffer;
char* write_p;
int filedesc;

struct Logger* logger; 

#define log_string(str, message, nbytes, ...) \
	do { \
		nbytes = snprintf(write_p, (uint8_t*)log_buffer[logger->cur_buf] + \
		logger->size_of_buf - (uint8_t*)write_p, str message "\n", ##__VA_ARGS__); \
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
				if (nbytes >= (uint8_t*)log_buffer[logger->cur_buf] + logger->size_of_buf - (uint8_t*)write_p) \
				{	\
					if (write_p == (char*)log_buffer[logger->cur_buf]) \
					{ \
						printf("Log message is too long"); \
						log_with_level(log_lev, nbytes, "log_message is too long");  \
					} \
					else \
					{	\
							stall_cur_buf(); \
							switch_buf(logger); \
							log_with_level(log_lev, nbytes, message, ##__VA_ARGS__); \
							write_p = (uint8_t*)(write_p) + nbytes; \
					} \
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

			
				  
