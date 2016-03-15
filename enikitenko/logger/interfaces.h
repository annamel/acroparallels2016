#ifndef __PLATFORM__
#define __PLATFORM__

#include <stdio.h>
#include "log.h"

#define MAX_BACKTRACE_SIZE 2048
#define FUNCTION_NAME_SIZE 512

#ifdef __cplusplus
extern "C"
{
#endif

unsigned gettid();
void set_console_color(FILE* file, LogLevel log_level);
void get_backtrace(char* str, int max_size);
void get_demangled_function_name(char* function_name, const char* input);

#ifdef __cplusplus
}
#endif

#endif // __PLATFORM__
