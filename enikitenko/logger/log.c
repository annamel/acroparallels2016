#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>

#include "log.h"
#include "interfaces.h"

typedef struct log_message_data
{
	struct log_message_data* prev;
	struct log_message_data* next;

	LogLevel level;
	char str[LOG_STRING_SIZE];

} log_message_data_t;

typedef struct
{
	bool working;
	LogLevel level;
	bool colors;
	FILE* file;

	pthread_t thread;
	pthread_mutex_t pool_mutex;
	pthread_mutex_t ready_messages_mutex;
	pthread_cond_t condition;

	log_message_data_t* message_data_pool;
	log_message_data_t* ready_messages_data_first;
	log_message_data_t* ready_messages_data_last;

} log_data_t;

log_data_t* log_data = NULL;

static void internal_print_log(LogLevel log_level, const char* str);

static void write_to_log(LogLevel log_level, const char* str)
{
	assert(log_level >= LOG_DEBUG && log_level <= LOG_ERROR);
	assert(str);

	if (log_data->level > log_level)
		return;

	FILE* file = (log_level >= STDERR_LOG_LEVEL) ? stderr : stdout;
	
	if (log_data->colors)
		set_console_color(file, log_level);
	fprintf(file, "%s\n", str);
	if (log_data->colors)
		set_console_color(file, INTERNAL_DEFAULT_COLOR);

	fflush(file);

	fprintf(log_data->file, "%s\n", str);
	fflush(log_data->file);
}

static void* logger_thread(void* data)
{
	bool finishing = false;

	while (!finishing)
	{
		int ret;
		ret = pthread_mutex_lock(&log_data->ready_messages_mutex);
		assert(!ret);

		while (!log_data->ready_messages_data_last)
		{
			pthread_cond_wait(&log_data->condition, &log_data->ready_messages_mutex);
			assert(!ret);
		}
		
		log_message_data_t* message = log_data->ready_messages_data_last;

		if (message->prev)
			message->prev->next = NULL;
		log_data->ready_messages_data_last = message->prev;
		if (!log_data->ready_messages_data_last)
			log_data->ready_messages_data_first = NULL;

		ret = pthread_mutex_unlock(&log_data->ready_messages_mutex);
		assert(!ret);

		if (message->level == INTERNAL_LOG_FINISH_MSG)
			break;

		write_to_log(message->level, message->str);

		ret = pthread_mutex_lock(&log_data->pool_mutex);
		assert(!ret);
		
		message->next = log_data->message_data_pool;
		log_data->message_data_pool = message;
	
		ret = pthread_mutex_unlock(&log_data->pool_mutex);
		assert(!ret);
	}

	return NULL;
}

void log_init(LogLevel log_level, bool colors)
{
	time_t seconds = time(NULL);
	struct tm* timeinfo = localtime(&seconds);

	char path[FILENAME_MAX] = "";
	strftime(path, FILENAME_MAX, "log [%d-%m-%y %H.%M.%S].txt", timeinfo);

	return log_init_file(log_level, colors, path);
}

static void malloc_message_pool_data()
{
	int i;
	log_message_data_t* prev = NULL;
	for (i = 0; i < LOG_MESSAGE_POOL_SIZE; i++)
	{
		log_data->message_data_pool = (log_message_data_t*) malloc(sizeof (log_message_data_t));
		assert(log_data->message_data_pool);
		log_data->message_data_pool->prev = prev;
		log_data->message_data_pool->next = NULL;
		if (prev)
			log_data->message_data_pool->next = prev;
		prev = log_data->message_data_pool;
	}
}

void log_init_file(LogLevel log_level, bool colors, const char* path)
{
	assert(!log_data);
	assert(path);
	assert(log_level >= LOG_DEBUG && log_level <= NO_LOGS);

	log_data = (log_data_t*) malloc(sizeof (log_data_t));
	assert(log_data);

	log_data->level = log_level;

	if (log_level == NO_LOGS)
		return;

	log_data->working = true;
	log_data->colors = colors;
	log_data->file = fopen(path, "w");
	
	assert(log_data->file);

	malloc_message_pool_data();

	log_data->ready_messages_data_first = NULL;
	log_data->ready_messages_data_last = NULL;

	int ret;
	ret = pthread_mutex_init(&log_data->pool_mutex, NULL);
	assert(!ret);
	ret = pthread_mutex_init(&log_data->ready_messages_mutex, NULL);
	assert(!ret);
	ret = pthread_cond_init(&log_data->condition, NULL);
	assert(!ret);

	ret = pthread_create(&log_data->thread, NULL, logger_thread, NULL);
	assert(!ret);

	log_info("LOG INITIALIZED");
}

void log_destroy()
{
	assert(log_data);

	if (log_data->level == NO_LOGS)
	{
		free(log_data);
		log_data = NULL;
		return;
	}

	log_info("LOG DESTROYED");

	internal_print_log(INTERNAL_LOG_FINISH_MSG, "");

	int ret = pthread_join(log_data->thread, NULL);
	assert(!ret);

	ret = pthread_mutex_destroy(&log_data->pool_mutex);
	assert(!ret);
	ret = pthread_mutex_destroy(&log_data->ready_messages_mutex);
	assert(!ret);

	fclose(log_data->file);

	while (log_data->message_data_pool)
	{
		log_message_data_t* next = log_data->message_data_pool->next;
		free(log_data->message_data_pool);
		log_data->message_data_pool = next;
	}

	free(log_data);
	log_data = NULL;
}

void print_log_va_args(LogLevel log_level, const char* format, ...)
{
	char str[LOG_STRING_SIZE] = "";
	va_list vl;
	va_start(vl, format);
	vsnprintf(str, LOG_STRING_SIZE, format, vl);
	va_end(vl);

	print_log(log_level, str);
}

void print_log_with_info_va_args(LogLevel log_level, const char* format, ...)
{
	char str[LOG_STRING_SIZE] = "";
	va_list vl;
	va_start(vl, format);
	vsnprintf(str, LOG_STRING_SIZE, format, vl);
	va_end(vl);

	print_log_with_info(log_level, str);
}

void print_log_with_source_info_va_args(const char* file, const char* function, 
	int line, LogLevel log_level, const char* format, ...)
{
	char str[LOG_STRING_SIZE] = "";
	va_list vl;
	va_start(vl, format);
	vsnprintf(str, LOG_STRING_SIZE, format, vl);
	va_end(vl);

	print_log_with_source_info(file, function, line, log_level, str);
}

void print_log_with_backtrace_va_args(const char* file, const char* function, 
	int line, LogLevel log_level, const char* format, ...)
{
	char str[LOG_STRING_SIZE] = "";
	va_list vl;
	va_start(vl, format);
	vsnprintf(str, LOG_STRING_SIZE, format, vl);
	va_end(vl);

	print_log_with_backtrace(file, function, line, log_level, str);
}

static void internal_print_log(LogLevel log_level, const char* str)
{
	assert(str);

	if (log_data->level > log_level)
		return;

	int ret;
	ret = pthread_mutex_lock(&log_data->pool_mutex);
	assert(!ret);
	log_message_data_t* message_data = log_data->message_data_pool;
	if (!message_data)
	{
		malloc_message_pool_data();
		message_data = log_data->message_data_pool;
	}
	assert(message_data);
	log_data->message_data_pool = log_data->message_data_pool->next;
	ret = pthread_mutex_unlock(&log_data->pool_mutex);
	assert(!ret);

	strncpy(message_data->str, str, LOG_STRING_SIZE);
	message_data->level = log_level;

	ret = pthread_mutex_lock(&log_data->ready_messages_mutex);
	assert(!ret);

	message_data->prev = NULL;
	message_data->next = log_data->ready_messages_data_first;
	if (log_data->ready_messages_data_first)
		log_data->ready_messages_data_first->prev = message_data;
	log_data->ready_messages_data_first = message_data;
	if (!log_data->ready_messages_data_last)
		log_data->ready_messages_data_last = message_data;

	ret = pthread_cond_signal(&log_data->condition);
	assert(!ret);
	ret = pthread_mutex_unlock(&log_data->ready_messages_mutex);
	assert(!ret);
}

void print_log(LogLevel log_level, const char* str)
{
	assert(log_data);
	assert(log_level >= LOG_DEBUG && log_level <= LOG_ERROR);
	if (log_data->level != NO_LOGS)
		internal_print_log(log_level, str);
}

#define LEVEL_STR_SIZE 16
void print_log_with_info(LogLevel log_level, const char* str)
{
	char ready_str[LOG_STRING_SIZE] = "";

	time_t seconds = time(NULL);
	struct tm* timeinfo = localtime(&seconds);
	int size = strftime(ready_str, LOG_STRING_SIZE, "[%H:%M:%S", timeinfo);

	char level_str[LEVEL_STR_SIZE] = "";
	switch (log_level)
	{
		case LOG_DEBUG:
			strncpy(level_str, "DEBUG", LEVEL_STR_SIZE);
			break;
		case LOG_INFO:
			strncpy(level_str, "INFO ", LEVEL_STR_SIZE);
			break;
		case LOG_WARNING:
			strncpy(level_str, "WARN ", LEVEL_STR_SIZE);
			break;
		case LOG_ERROR:
			strncpy(level_str, "ERROR", LEVEL_STR_SIZE);
			break;
		default:
			abort();
	}
	snprintf(&ready_str[size], LOG_STRING_SIZE - size, "|%s|%5u] %s", level_str, gettid(), str);

	print_log(log_level, ready_str);
}

void print_log_with_source_info(const char* file, const char* function,
	int line, LogLevel log_level, const char* str)
{
	char ready_str[LOG_STRING_SIZE] = "";

	snprintf(ready_str, LOG_STRING_SIZE, "At function %s (%s:%d): %s", function, file, line, str);

	print_log_with_info(log_level, ready_str);
}

void print_log_with_backtrace(const char* file, const char* function,
	int line, LogLevel log_level, const char* str)
{
	char ready_str[LOG_STRING_SIZE] = "\nBacktrace:\n";
	int str_size = strlen(ready_str);
	get_backtrace(&ready_str[str_size], LOG_STRING_SIZE - str_size);

	str_size = strlen(ready_str);
	if (str_size < LOG_STRING_SIZE)
		strncpy(&ready_str[str_size], str, LOG_STRING_SIZE - str_size);

	print_log_with_source_info(file, function, line, log_level, ready_str);
}
