#ifndef __LOG__
#define __LOG__

#include "logger.h"



#ifndef _LOG_FLUSH
#define _LOG_FLUSH false
#endif

#ifndef _LOG_MAX_STR
#define _LOG_MAX_STR 512
#endif

#ifdef _LOG_ENABLED
#define LOG(level, fmt, ...) log_write(_LOG_FLUSH, level, fmt, ##__VA_ARGS__)
#else
#define  LOG(level, fmt, ...) do {} while (false)
#endif

#define LOGI(fmt, ...) LOG(LOGGER_LEVEL_INFO,    fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) LOG(LOGGER_LEVEL_WARNING, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) LOG(LOGGER_LEVEL_DEBUG,   fmt, ##__VA_ARGS__)
#define LOGV(fmt, ...) LOG(LOGGER_LEVEL_VERBOSE, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOG(LOGGER_LEVEL_ERROR,   fmt, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C"
{
#endif
	void init_logger_client(const char* name);
	void log_write(bool flush, int level, const char* fmt, ...);
	
#ifdef __cplusplus
}
#endif

#endif
