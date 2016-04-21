#define _LOG_FLUSH 1

#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/syscall.h>

static logger_t logger = {.initialized = false, .mutex = PTHREAD_MUTEX_INITIALIZER};

void init_logger_client(const char* name)
{
	pthread_mutex_lock(&logger.mutex);
	
	if(logger.initialized)
	{
		pthread_mutex_unlock(&logger.mutex);
		return;
	}
	
	if (!name)
		name = getenv("_");
	
	char _name[80] = "";
	if (!name)
	{
		sprintf(_name, "%d", (int) getpid());
		name = _name;
	}
		
	init_logger(&logger, false, 0, 0, 0, 0);
	strncpy(logger.name, name, LOGGER_NAME_MAX);
	
	pthread_mutex_unlock(&logger.mutex);
}

void log_write(bool flush, int level, const char* fmt, ...)
{
	if (!logger.initialized)
		init_logger_client(NULL);
	
	pthread_mutex_lock(&logger.mutex);	
	assert(_LOG_MAX_STR < logger.buffers->buffer_size);
		
	if (logger.buffer == -1)
	{
		logger.buffer = dequeue_log_buffer(&logger.free_buffers, logger.buffers->buffers);
		map_buffer(&logger);
		logger.curr_position = 0;	
		logger.buffer_data[0] = 0;
	}
	
	if (level >= logger.buffers->log_level)
	{
		const char* prefix = "\x1B[0m[V]";
		switch (level)
		{
			case LOGGER_LEVEL_VERBOSE:
				prefix ="\x1B[34m[?]";
				break;
			case LOGGER_LEVEL_INFO:
				prefix = "\x1B[0m[I]";
				break;
			case LOGGER_LEVEL_DEBUG:
				prefix = "\x1B[33m[D]";
				break;
			case LOGGER_LEVEL_WARNING:
				prefix = "\x1B[33m[W]";
				break;
			case LOGGER_LEVEL_ERROR:
				prefix = "\x1B[31m[E]";
				break;
		}
		
		va_list vp;
		va_start(vp, fmt);
		
		int len = _LOG_MAX_STR - 1;
		

		snprintf(logger.buffer_data + logger.curr_position, len, "%s[%s(%d)][%d] ", prefix, logger.name, (int) getpid(), (int) syscall(SYS_gettid));
		int written = strlen(logger.buffer_data + logger.curr_position);
		len -= written;
		logger.curr_position += written;
		vsnprintf(logger.buffer_data + logger.curr_position, len, fmt, vp);
		logger.curr_position += strlen(logger.buffer_data + logger.curr_position);
	}
	
	if (flush || logger.curr_position + _LOG_MAX_STR >= (int) logger.buffers->buffer_size)
		if (logger.buffer != -1)
		{
			unmap_buffer(&logger);
			queue_log_buffer(&logger.full_buffers, logger.buffers->buffers, logger.buffer);
			logger.buffer = -1;
		}
		
	pthread_mutex_unlock(&logger.mutex);
}
