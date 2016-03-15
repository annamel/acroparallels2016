#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <execinfo.h>
#include <dlfcn.h>

#include "interfaces.h"
#include "log.h"

typedef enum
{
	FG_DEFAULT = 39, 
	FG_BLACK = 30, 
	FG_RED = 31,
	FG_GREEN = 32, 
	FG_YELLOW = 33, 
	FG_BLUE = 34, 
	FG_MAGENTA = 35, 
	FG_CYAN = 36, 
	FG_LIGHT_GRAY = 37, 
	FG_DARK_GRAY = 90, 
	FG_LIGHT_RED = 91, 
	FG_LIGHT_GREEN = 92, 
	FG_LIGHT_YELLOW = 93, 
	FG_LIGHT_BLUE = 94, 
	FG_LIGHT_MAGENTA = 95, 
	FG_LIGHT_CYAN = 96, 
	FG_WHITE = 97
} ColorCode;

#define LOG_DEFAULT_COLOR FG_DEFAULT
#define LOG_DEBUG_COLOR FG_DEFAULT
#define LOG_INFO_COLOR FG_GREEN
#define LOG_WARNING_COLOR FG_YELLOW
#define LOG_ERROR_COLOR FG_RED

static void set_console_color_code(FILE* file, ColorCode color_code)
{
	fprintf(file, "\033[%dm", (int) color_code);
}

void set_console_color(FILE* file, LogLevel log_level)
{
	switch (log_level)
	{
		case LOG_DEBUG:
			set_console_color_code(file, LOG_DEBUG_COLOR);
			break;

		case LOG_INFO:
			set_console_color_code(file, LOG_INFO_COLOR);
			break;

		case LOG_WARNING:
			set_console_color_code(file, LOG_WARNING_COLOR);
			break;

		case LOG_ERROR:
			set_console_color_code(file, LOG_ERROR_COLOR);
			break;

		case INTERNAL_DEFAULT_COLOR:
			set_console_color_code(file, LOG_DEFAULT_COLOR);
			break;

		default:
			abort();
	}
}

unsigned gettid()
{
	return (unsigned) syscall(SYS_gettid);
}

void get_backtrace(char* str, int max_size)
{
	void* buffer[MAX_BACKTRACE_SIZE];
	int size = backtrace(buffer, MAX_BACKTRACE_SIZE);
	int i;

	for (i = 0; i < size; i++)
	{
		Dl_info info;
		int error_code = dladdr(buffer[i], &info);

		int str_size = strlen(str);
		if (error_code)
		{
			if (!info.dli_sname)
				snprintf(&str[str_size], max_size - str_size, "%s(unknown) [%p]\n", 
					info.dli_fname, buffer[i]);
			else
			{
				char function_name[FUNCTION_NAME_SIZE] = "";
				get_demangled_function_name(function_name, info.dli_sname);

				snprintf(&str[str_size], max_size - str_size, "%s(%s+0x%td) [%p]\n", 
					info.dli_fname, function_name, (char*) buffer[i] - (char*) info.dli_saddr, buffer[i]);
			}
		}
		else
			snprintf(&str[size], max_size - size, "invalid address %p\n", buffer[i]);
	}
}
