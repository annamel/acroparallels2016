#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <semaphore.h>

#define BUFF_SIZE 100
#define DEFAULT_LOGFILE "logfile.txt"
#define DEFAULT_LOG_LEVEL WARNING
#define CALL_STACK_SIZE 50

typedef struct logger logger_t;
typedef struct circular_buf circular_buff_t;
typedef enum log_level log_level;

enum log_level {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    NUM_OF_OPTIONS
};

struct logger {
    FILE* file;
    circular_buff_t* buffer;
    log_level default_log_level;
    sem_t sem_loc;
};

struct circular_buf {
    char *buffer[BUFF_SIZE];
    unsigned int head;
    unsigned int tail;
};

logger_t *log_init();
int log_write_in_file(log_level log_level, const char *restrict message, ...);
int log_write(log_level log_level, const char *restrict message, ...);
int log_deinit();
int log_set_default_loglvl(log_level loglvl);
int log_set_logfile(const char *filename);

#endif // LOGGER_H
