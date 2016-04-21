//*********************************************************
//********                                       **********
//********      Created by Alexander Snorkin     **********
//********              22.03.2016               **********
//********                                       **********
//*********************************************************

#include "logger.h"



logger_t *logger_init(char *filename)
{
    if(LOGGER_SINGLETON != NULL)
        return LOGGER_SINGLETON;

    LOGGER_SINGLETON = (logger_t *)calloc(1, sizeof(logger_t));
    if(LOGGER_SINGLETON == NULL)
    {
        perror("Logger initialization has failed\n");
        return NULL;
    }

    if(filename == NULL)
        filename = DEFAULT_LOGFILE;

    if((LOGGER_SINGLETON->file = fopen(filename, "w")) == NULL)
    {
        perror("Logger initialization has failed\n");
        return NULL;
    }

    buff_init();

    LOGGER_SINGLETON->default_log_level = DEBUG;

    return LOGGER_SINGLETON;
}



int set_default_log_level(log_lvl_t new_default_loglvl)
{
    if(LOGGER_SINGLETON == NULL)
    {
        perror("Setting of default log level has failed: logger is NULL\n");
        return -1;
    }
    else if(new_default_loglvl > FATAL)
    {
        errno = EINVAL;
        perror("Setting of default log level has failed");
        return -2;
    }

    LOGGER_SINGLETON->default_log_level = new_default_loglvl;

    return 0;
}



int set_logfile(char *filename)
{
    if(LOGGER_SINGLETON == NULL)
        return -1;
    else if(filename == NULL)
        return -2;

    if(!fclose(LOGGER_SINGLETON->file))
    {
        if((LOGGER_SINGLETON->file = fopen(filename, "w")) == NULL)
        {
            perror("Logfile setting has failed\n");
            return -4;
        }

        return 0;
    }
    else
        return -3;
}



int log_write_in_logfile(char *message, log_lvl_t log_level)
{
    if(LOGGER_SINGLETON == NULL)
    {
        perror("Writing in the logfile has failed: logger is NULL\n");
        return -1;
    }
    else if(message == NULL)
    {
        errno = EINVAL;
        perror("Setting of default log level has failed");
        return -2;
    }
    else if(log_level > FATAL)
    {
        errno = EINVAL;
        perror("Setting of default log level has failed");
        return -3;
    }

    if(log_level >= LOGGER_SINGLETON->default_log_level)
    {
        fprintf(LOGGER_SINGLETON->file, "%s: %s\n",
                LOG_LEVELS[log_level], message);
        if(log_level == FATAL)
        {
            fflush(LOGGER_SINGLETON->file);
            //TODO:  Write the call stack
            fflush(LOGGER_SINGLETON->file);
            logger_deinit();
            exit(EXIT_FAILURE);
        }
        else
            return 0;
    }
    else
    {
        return -4;
    }
}



int log_write(char *message, log_lvl_t log_level)
{
    if(LOGGER_SINGLETON == NULL)
    {
        perror("Writing in the logfile has failed: logger is NULL\n");
        return -1;
    }
    else if(message == NULL)
    {
        errno = EINVAL;
        perror("Setting of default log level has failed");
        return -2;
    }
    else if(log_level > FATAL)
    {
        errno = EINVAL;
        perror("Setting of default log level has failed");
        return -3;
    }
    else if(log_level < LOGGER_SINGLETON->default_log_level)
        return -4;

    if(!buff_is_full() && log_level < FATAL)
    {
        LOGGER_SINGLETON->buffer->buffer[LOGGER_SINGLETON->buffer->head] =
                create_message(message, log_level);
        LOGGER_SINGLETON->buffer->head ++;

        return 0;
    }
    else if(log_level < FATAL)   // If file is open
    {                            //TODO: check it
        int i = 0;
        for(i = 0; i < BUFF_SIZE; i ++)
        {
            fputs(LOGGER_SINGLETON->buffer->buffer[i],
                  LOGGER_SINGLETON->file);
            LOGGER_SINGLETON->buffer->buffer[i] = "";
        }

        LOGGER_SINGLETON->buffer->head = 0;
        LOGGER_SINGLETON->buffer->tail = 0;

        LOGGER_SINGLETON->buffer->buffer[LOGGER_SINGLETON->buffer->head] =
                create_message(message, log_level);
        LOGGER_SINGLETON->buffer->head ++;

        return 0;
    }
    else
    {
       fputs(create_message(message, log_level), LOGGER_SINGLETON->file);
       //TODO: callstack

       int i = 0;
       for(i = 0; i < BUFF_SIZE; i ++)
       {
           fputs(LOGGER_SINGLETON->buffer->buffer[i],
                 LOGGER_SINGLETON->file);
           LOGGER_SINGLETON->buffer->buffer[i] = "";
       }

       logger_deinit();

       exit(EXIT_FAILURE);
    }
}




void logger_deinit()
{
    return;
}





int buff_init()
{
    if(LOGGER_SINGLETON == NULL)
    {
        perror("Buffer initialization has failed: logger is NULL\n");
        return -1;
    }

    LOGGER_SINGLETON->buffer = (ring_buff_t *)calloc(1, sizeof(ring_buff_t));
    if(LOGGER_SINGLETON->buffer == NULL)
    {
        perror("Buffer initialization has failed\n");
        return -2;
    }

    LOGGER_SINGLETON->buffer->head = 0;
    LOGGER_SINGLETON->buffer->tail = 0;

    int i = 0;
    for(i = 0; i < BUFF_SIZE; i++)
        LOGGER_SINGLETON->buffer->buffer[i] = "";

    return 0;
}



int buff_is_full()
{
    if(LOGGER_SINGLETON == NULL)
        return -1;

    if((LOGGER_SINGLETON->buffer->tail -
        LOGGER_SINGLETON->buffer->head == 1) ||
            ((LOGGER_SINGLETON->buffer->head == BUFF_SIZE - 1) &&
             (LOGGER_SINGLETON->buffer->tail == 0)))
        return 1;
    else
        return 0;
}



char *create_message(char *message, log_lvl_t log_level)
{
    char *log_message = (char *)calloc(strlen(message)
                                       + 10, sizeof(char));
    strcat(log_message, LOG_LEVELS[log_level]);
    strcat(log_message, message);

    free(message);

    return log_message;
}
