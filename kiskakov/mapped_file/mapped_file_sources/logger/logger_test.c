#include <stdio.h>
#include <unistd.h>
#include <pthread/pthread.h>
#include <stdlib.h>
#include <execinfo.h>

#include "logger.h"

static void * thread_func(void * arg)
        {
        if (*(int *)arg == 7)
                log_write(LOG_FATAL, "FATAL from 7");

        for (int j = 0; j < 10; j++)
                {
                log_write(*(int *)arg % 2 ? LOG_ERROR : LOG_FATAL, "I'm thread #%d", *(int *)arg);
                }

        return NULL;
        }

int main()
        {
        log_init(LOG_DEBUG, "/tmp/test.log");

        int i = 0;

        pthread_t threads[10];
        int indices[10];
        for (i = 0; i < 10; i++)
                {
                indices[i] = i;
                }

        for (i = 0; i < 10; i++)
                {
                pthread_create(&threads[i], NULL, thread_func, &indices[i]);
                }


        for (i = 0; i < 10; i++)
                {
                pthread_join(threads[i], NULL);
                }


        //backtrace_symbols_fd(buffer, calls, logger.file);



        log_deinit();
        }

