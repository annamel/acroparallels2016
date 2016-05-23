#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>



#include "logger.h"



//  String prefix of logging levels for log messages
const char *LOG_LEVELS[COUNT_OF_LOGLEVELS] = {"DEBUG::",
                                              "INFO::",
                                              "WARNING::",
                                              "ERROR::",
                                              "FATAL::"};



//************************************************
//**  GLOBAL CONSTANT !!! SINGLETON !!! LOGGER ***
//**                                           ***
         logger_t *LOGGER_SING = NULL;         //*
//************************************************



static int buff_init();
static int buff_add_msg(char *__restrict message, log_lvl_t log_level);
static int buff_fflush();
static int buff_is_full();
static int buff_deinit();
static int write_callstack_in_logfile();
static char *create_message(char *__restrict message, log_lvl_t log_level);



logger_t *logger_init()
{
    if(LOGGER_SING)
        return LOGGER_SING;

    LOGGER_SING = (logger_t *)calloc(1, sizeof(logger_t));
    if(!LOGGER_SING)
        return NULL;

    LOGGER_SING->file = fopen(DEFAULT_LOGFILE, "a");
    if(!LOGGER_SING->file)
        return NULL;

    if(buff_init())
        return NULL;

    LOGGER_SING->default_log_level = DEFAULT_LOGLEVEL;

    return LOGGER_SING;
}



int log_write_in_logfile(log_lvl_t log_level, const char *__restrict message, ...)
{
    LOGGER_SING = logger_init();
    if(!LOGGER_SING)
        return EAGAIN;

    if(log_level < LOGGER_SING->default_log_level)
        return 0;

    if(!message || log_level > FATAL || log_level < DEBUG)
        return EINVAL;    

    if(fprintf(LOGGER_SING->file, "%s%s\n",
            LOG_LEVELS[log_level], message) < 0)
        return ETXTBSY;

    if(log_level == FATAL)
    {
        int error = 0;

        error = write_callstack_in_logfile();
        if(error) return error;

        if(fflush(LOGGER_SING->file))
            return ETXTBSY;

        error = logger_deinit();
        if(error) return error;

        exit(EXIT_FAILURE);
    }
    else
        return 0;

    return 0;
}



int log_write(log_lvl_t log_level, const char *__restrict message, ...)
{    
    LOGGER_SING = logger_init();
    if(!LOGGER_SING)
        return EAGAIN;

    if(log_level < LOGGER_SING->default_log_level)
        return 0;

    int error = 0;

    if(!buff_is_full() && log_level < FATAL)
        return buff_add_msg(message, log_level);
    else if(log_level < FATAL)
    {
        error = buff_fflush();
        if(error) return error;

        return buff_add_msg(message, log_level);
    }
    else
    {
        if(fputs(create_message(message, log_level),
                 LOGGER_SING->file) == EOF)
            return ETXTBSY;

        error = write_callstack_in_logfile();
        if(error)
            return error;

        if(fputs("***Buffer history***\n", LOGGER_SING->file) == EOF)
            return ETXTBSY;

        error = logger_deinit();

        exit(EXIT_FAILURE);
    }

    return 0;
}



int logger_deinit()
{
    if(!LOGGER_SING)
        return 0;

    int error = 0;

    error = buff_fflush();
    if(error) return error;

    error = buff_deinit();
    if(error) return error;

    if(fclose(LOGGER_SING->file))
        return ETXTBSY;

    free(LOGGER_SING);

    return 0;
}



int log_set_default_loglvl(log_lvl_t new_default_loglvl)
{
    LOGGER_SING = logger_init();
    if(!LOGGER_SING)
        return EAGAIN;

    if(new_default_loglvl < DEBUG || new_default_loglvl > FATAL)
        return EINVAL;

    LOGGER_SING->default_log_level = new_default_loglvl;

    return 0;
}



int log_set_logfile(const char *filename)
{
    LOGGER_SING = logger_init();
    if(!LOGGER_SING)
        return EAGAIN;

    if(fclose(LOGGER_SING->file))
        return ETXTBSY;

    if((LOGGER_SING->file = fopen(filename, "a")) == NULL)
        return ETXTBSY;

        return 0;
}



static int write_callstack_in_logfile()
{
    char **strings;
    void *backtrace_buffer[CALL_STACK_SIZE];

    int nptrs = backtrace(backtrace_buffer, CALL_STACK_SIZE);
    strings = backtrace_symbols(backtrace_buffer, nptrs);

    if(fprintf(LOGGER_SING->file, "### STACK TRACE ###\n") < 0)
        return ETXTBSY;

    for (int j = 0; j < nptrs; j++)
        if(fprintf(LOGGER_SING->file, "%s\n", strings[j]) < 0)
            return ETXTBSY;

    if(fprintf(LOGGER_SING->file, "###################\n") < 0)
        return ETXTBSY;

    return 0;
}



static char *create_message(char *__restrict message, log_lvl_t log_level)
{
    char *log_message = (char *)calloc(strlen(message)
                                       + 10, sizeof(char));
    if(!log_message)
        return NULL;
    strcat(log_message, LOG_LEVELS[log_level]);
    strcat(log_message, message);
    strcat(log_message, "\n");

    return log_message;
}
//*****************************************************************************
//*******                                                            **********
//*******                   Ring buffer functions                    **********
//*******                                                            **********
//*****************************************************************************
static int buff_init()
{
    LOGGER_SING->buffer = (ring_buff_t *)calloc(1, sizeof(ring_buff_t));
    if(!LOGGER_SING->buffer)
        return ENOMEM;

    LOGGER_SING->buffer->head = 0;
    LOGGER_SING->buffer->tail = 0;

    for(int i = 0; i < BUFF_SIZE; i++)
        LOGGER_SING->buffer->buffer[i] = "";

    return 0;
}


//  returns 1 if buffer is full
//  else 0
static int buff_is_full()
{
    unsigned int tail = LOGGER_SING->buffer->tail;
    unsigned int head = LOGGER_SING->buffer->head;

    if( (tail - head == 1) || (head == BUFF_SIZE - 1 && tail == 0) )
        return 1;
    else
        return 0;
}



static int buff_add_msg(char *__restrict message, log_lvl_t log_level)
{
    if(!message || log_level > FATAL || log_level < DEBUG)
        return EINVAL;

    LOGGER_SING->buffer->buffer[LOGGER_SING->buffer->head] =
            create_message(message, log_level);
    LOGGER_SING->buffer->head =
            (LOGGER_SING->buffer->head + 1) % BUFF_SIZE;

    if(LOGGER_SING->buffer->head == LOGGER_SING->buffer->tail)
        LOGGER_SING->buffer->tail =
                (LOGGER_SING->buffer->tail + 1) % BUFF_SIZE;

    return 0;
}



static int buff_fflush()
{
    for(unsigned int i = LOGGER_SING->buffer->tail; i < BUFF_SIZE; i++)
    {
        if(fputs(LOGGER_SING->buffer->buffer[i],
              LOGGER_SING->file) == EOF)
            return ETXTBSY;

        free(LOGGER_SING->buffer->buffer[i]);
        LOGGER_SING->buffer->buffer[i] = "";
    }
    for(unsigned int i = 0; i < LOGGER_SING->buffer->head; i++)
    {
        if(fputs(LOGGER_SING->buffer->buffer[i],
              LOGGER_SING->file) == EOF)
            return ETXTBSY;

        free(LOGGER_SING->buffer->buffer[i]);
        LOGGER_SING->buffer->buffer[i] = "";
    }

    LOGGER_SING->buffer->head = 0;
    LOGGER_SING->buffer->tail = 0;

    return 0;
}



static int buff_deinit()
{
    free(LOGGER_SING->buffer);
    return 0;
}
