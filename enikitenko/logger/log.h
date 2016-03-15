#ifndef __LOG__
#define __LOG__

#include <stdbool.h>

typedef enum
{
	LOG_DEBUG, 
	LOG_INFO, 
	LOG_WARNING, 
	LOG_ERROR, 
	NO_LOGS, 

	LOG_INTERNAL
} LogLevel;

#define LOG_ALL LOG_DEBUG
#define INTERNAL_LOG_FINISH_MSG LOG_INTERNAL
#define INTERNAL_DEFAULT_COLOR LOG_INTERNAL

#define STDERR_LOG_LEVEL LOG_WARNING
#define LOG_STRING_SIZE 8192
#define LOG_MESSAGE_POOL_SIZE 32

#ifdef __cplusplus
extern "C"
{
#endif

void log_init(LogLevel log_level, bool colors);
void log_init_file(LogLevel log_level, bool colors, const char* path);
void log_destroy();

void print_log(LogLevel log_level, const char* str);
void print_log_with_info(LogLevel log_level, const char* str);
void print_log_with_source_info(const char* file, const char* function,
	int line, LogLevel log_level, const char* str);
void print_log_with_backtrace(const char* file, const char* function, 
	int line, LogLevel log_level, const char* str);

void print_log_va_args(LogLevel log_level, const char* format, ...);
void print_log_with_info_va_args(LogLevel log_level, const char* format, ...);
void print_log_with_source_info_va_args(const char* file, const char* function, 
	int line, LogLevel log_level, const char* format, ...);
void print_log_with_backtrace_va_args(const char* file, const char* function, 
	int line, LogLevel log_level, const char* format, ...);

#ifdef __cplusplus
}
#endif

#define log_debug(...)		print_log_with_info_va_args(LOG_DEBUG, __VA_ARGS__)
#define log_info(...)		print_log_with_info_va_args(LOG_INFO, __VA_ARGS__)
#define log_warning(...)	print_log_with_info_va_args(LOG_WARNING, __VA_ARGS__)
#define log_error(...)		print_log_with_backtrace_va_args(__FILE__, __FUNCTION__, __LINE__, LOG_ERROR, __VA_ARGS__)

#define logd log_debug
#define logi log_info
#define logw log_warning
#define loge log_error

#endif // __LOG__
