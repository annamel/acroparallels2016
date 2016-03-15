#ifndef __LOG_H__
#define __LOG_H__

#define LOG_FATAL   0
#define LOG_ERR		1
#define LOG_WARN	2
#define LOG_INFO	3
#define LOG_DEBUG	4

#define DEFAULT_LOG_PATH "log"
#define DEFAULT_LOG_LEVEL LOG_DEBUG

#ifdef LOG_ON

#ifndef __GNUC__
#error "GCC compiler required"
#endif

void log_fini(void);
void log_init(const char * logfile, int lvl);
void log_write(int lvl, const char * fmt, ...) __attribute__((format(printf, 2, 3)));

#else

#define log_write(lvl, fmt...)
#define log_init(logfile, lvl)
#define log_fini()

#endif

#endif
