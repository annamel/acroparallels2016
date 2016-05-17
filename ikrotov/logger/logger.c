#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "logger.h"

const char *log_option_message[NUM_OF_OPTIONS] = {"DEBUG - ",
                                         "INFO - ",
                                         "WARNING - ",
                                         "ERROR - ",
                                         "FATAL - "};

logger_t *logger = NULL;

static int buff_init();
static int buff_add_msg(char *restrict msg, log_level log_level);
static int buff_fflush();
static int buff_is_full();
static int buff_deinit();
static int write_callstack();
static char *create_msg(char *restrict msg, log_level log_level);

logger_t *log_init() {
    if(logger)
        return logger;

    logger = (logger_t *)calloc(1, sizeof(logger_t));
    if(!logger)
        return NULL;

    logger->file = fopen(DEFAULT_LOGFILE, "a");
    if(!logger->file)
        return NULL;

    logger->default_log_level = DEFAULT_LOG_LEVEL;

    if(buff_init())
        return NULL;

    if(sem_init(&(logger->sem_loc), 0, 1))
        return NULL;

    return logger;
}

int log_write_in_file(log_level log_level, const char *restrict message, ...) {
    logger = log_init();
    if(!logger)
        return EAGAIN;

    if(!message || log_level > FATAL || log_level < DEBUG)
        return EINVAL;

    if(log_level < logger->default_log_level)
        return 0;

    sem_wait(&(logger->sem_loc));

    if(fprintf(logger->file, "%s%s\n", log_option_message[log_level], message) < 0) {
        sem_post(&(logger->sem_loc));
        return ETXTBSY;
    }

    if(log_level == FATAL)
    {
        int error = 0;

        error = write_callstack();
        if(error) {
            sem_post(&(logger->sem_loc));
            return error;
        }

        if(fflush(logger->file)){
            sem_post(&(logger->sem_loc));
            return ETXTBSY;
        }

        error = log_deinit();
        if(error) {
            sem_post(&(logger->sem_loc));
            return error;
        }

        sem_post(&(logger->sem_loc));
        exit(EXIT_FAILURE);
    }
    else {
        sem_post(&(logger->sem_loc));
        return 0;
    }

    sem_post(&(logger->sem_loc));
    return 0;
}

int log_write(log_level log_level, const char *restrict message, ...) {
    logger = log_init();
    if(!logger)
        return EAGAIN;

    if(log_level < logger->default_log_level)
        return 0;

    int error = 0;

    sem_wait(&(logger->sem_loc));

    if(!buff_is_full() && log_level < FATAL) {
        sem_post(&(logger->sem_loc));
        return buff_add_msg(message, log_level);
    }
    else if(log_level < FATAL) {
        error = buff_fflush();
        if(error) {
            sem_post(&(logger->sem_loc));
            return error;
        }

        sem_post(&(logger->sem_loc));
        return buff_add_msg(message, log_level);
    }
    else
    {
        if(fputs(create_msg(message, log_level), logger->file) == EOF) {
            sem_post(&(logger->sem_loc));
            return ETXTBSY;
        }

        error = write_callstack();
        if(error) {
            sem_post(&(logger->sem_loc));
            return error;
        }

        if(fputs("Buffer \n", logger->file) == EOF) {
            sem_post(&(logger->sem_loc));
            return ETXTBSY;
        }

        error = log_deinit();
        sem_post(&(logger->sem_loc));
        exit(EXIT_FAILURE);
    }

    sem_post(&(logger->sem_loc));
    return 0;
}

int log_deinit() {
    if(!logger)
        return 0;

    int error = 0;

    sem_wait(&(logger->sem_loc));

    error = buff_fflush();
    if(error) {
        sem_post(&(logger->sem_loc));
        return error;
    }

    error = buff_deinit();
    if(error) {
        sem_post(&(logger->sem_loc));
        return error;
    }

    if(fclose(logger->file)) {
        sem_post(&(logger->sem_loc));
        return ETXTBSY;
    }

    free(logger);

    sem_post(&(logger->sem_loc));
    return 0;
}

int log_set_default_loglvl(log_level new_default_loglvl) {
    logger = log_init();
    if(!logger)
        return EAGAIN;

    if(new_default_loglvl < DEBUG || new_default_loglvl > FATAL)
        return EINVAL;

    sem_wait(&(logger->sem_loc));
    logger->default_log_level = new_default_loglvl;

    sem_post(&(logger->sem_loc));
    return 0;
}

int log_set_logfile(const char *filename) {
    logger = log_init();
    if(!logger)
        return EAGAIN;

    sem_wait(&(logger->sem_loc));
    if(fclose(logger->file)) {
        sem_post(&(logger->sem_loc));
        return ETXTBSY;
    }

    if((logger->file = fopen(filename, "a")) == NULL) {
        sem_post(&(logger->sem_loc));
        return ETXTBSY;
    }

    sem_post(&(logger->sem_loc));
    return 0;
}

static int write_callstack() {
    char **strings;
    void *backtrace_buffer[CALL_STACK_SIZE];

    int nptrs = backtrace(backtrace_buffer, CALL_STACK_SIZE);
    strings = backtrace_symbols(backtrace_buffer, nptrs);

    if(fprintf(logger->file, " Stack Trace: \n") < 0)
        return ETXTBSY;

    for (int j = 0; j < nptrs; j++)
        if(fprintf(logger->file, "%s\n", strings[j]) < 0)
            return ETXTBSY;

    return 0;
}

static char *create_msg(char *restrict message, log_level log_level) {
    char *log_message = (char *)calloc(strlen(message)
                                       + 10, sizeof(char));
    strcat(log_message, log_option_message[log_level]);
    strcat(log_message, message);
    strcat(log_message, "\n");

    return log_message;
}

static int buff_init() {
    logger->buffer = (circular_buff_t *)calloc(1, sizeof(circular_buff_t));
    if(!logger->buffer)
        return ENOMEM;

    logger->buffer->head = 0;
    logger->buffer->tail = 0;

    for(int i = 0; i < BUFF_SIZE; i++)
        logger->buffer->buffer[i] = "";

    return 0;
}

static int buff_is_full() {
    unsigned int tail = logger->buffer->tail;
    unsigned int head = logger->buffer->head;

    if( (tail - head == 1) || (head == BUFF_SIZE - 1 && tail == 0) )
        return 1;
    else
        return 0;
}

static int buff_add_msg(char *restrict message, log_level log_level) {
    if(!message || log_level > FATAL || log_level < DEBUG)
        return EINVAL;

    logger->buffer->buffer[logger->buffer->head] =
            create_msg(message, log_level);
    logger->buffer->head =
            (logger->buffer->head + 1) % BUFF_SIZE;

    if(logger->buffer->head == logger->buffer->tail)
        logger->buffer->tail =
                (logger->buffer->tail + 1) % BUFF_SIZE;

    return 0;
}

static int buff_fflush() {
    for(int i = logger->buffer->tail; i < BUFF_SIZE; i++)
    {
        if(fputs(logger->buffer->buffer[i],
              logger->file) == EOF)
            return ETXTBSY;

        logger->buffer->buffer[i] = "";
    }

    for(int i = 0; i < logger->buffer->head; i++)
    {
        if(fputs(logger->buffer->buffer[i],
              logger->file) == EOF)
            return ETXTBSY;

        logger->buffer->buffer[i] = "";
    }

    logger->buffer->head = 0;
    logger->buffer->tail = 0;

    return 0;
}

static int buff_deinit() {
    free(logger->buffer);
    return 0;
}
