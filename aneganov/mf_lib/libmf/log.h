#ifndef __MF_LOG_H__
#define __MF_LOG_H__

#define LOG_FATAL   0
#define LOG_ERR     1
#define LOG_WARN    2
#define LOG_INFO    3
#define LOG_DEBUG   4

#ifdef LOG_ON

void log_init(void) __attribute__((constructor));
void log_fini(void) __attribute__((destructor));
int log_write(int lvl, const char * fmt, ...) __attribute__((format(printf, 2, 3)));
int log_configure(const char * logfile, int lvl);

#else

#define log_write(lvl, fmt...)
#define log_configure(logfile, lvl)

#endif

#endif
