#include <stdio.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <execinfo.h>
#include <pthread.h>

#include "logger.h"
#include "ring_buffer/ring_buffer.h"

#define LOG_ON

#define LOGGER_BUFFER_SIZE RING_BUFFER_SIZE

struct logger
        {
        log_level_t log_filter_level;

        ring_buffer_t * ring_buffer;

        int file; // log file fd

        pthread_t      dumping_thread;
        pthread_cond_t dumping_cond;
        };

struct log_line
        {
        log_level_t log_level;

        char message[LOG_MAX_MSG_LEN + 1];
        };

// Internal functions >>>

static void * _logger_dumping_thread(void * fd);
ret_code_t _dump_to_file(log_line_t ** buffer, char * buffer_to_dump, uint32_t n_lines);

// >>>

logger_t logger;

static volatile int logger_constructed = 0;
static volatile int logger_initialized = 0;

pthread_mutex_t tmp_mutex = PTHREAD_MUTEX_INITIALIZER; // Used only for dumping_cond


ret_code_t log_init(log_level_t log_filter_level, const char * log_file_path)
        {
        #ifndef LOG_ON
        return SUCCESS;
        #endif

        if (!__sync_bool_compare_and_swap(&logger_constructed, 0, 1))
                {
                // logger is already constructed
                fprintf(stderr, "logger: failed to construct and init logger. Reason: logger is already constructed\n");
                return ERROR;
                }

        int ret = 0;

        logger.log_filter_level = log_filter_level;

        ret = ring_buffer_construct(&logger.ring_buffer, sizeof(log_line_t));
        if (ret)
                {
                fprintf(stderr, "logger: failed to construct ring buffer\n");
                goto error_ring_buffer_construct;
                }

        logger.file = open(log_file_path, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);
        if (logger.file == -1)
                {
                fprintf(stderr, "logger: failed to open file [%s]. Reason: %s\n", log_file_path, strerror(errno));
                goto error_open_file;
                }

        ret = pthread_cond_init(&logger.dumping_cond, NULL);
        if (ret)
                {
                errno = ret;
                printf ("Failed to initialize dumping cond.\nReason: %s.\n", strerror (errno));
                goto error_cond_init;
                }

        logger_initialized = 1;

        ret = pthread_create(&logger.dumping_thread, NULL, _logger_dumping_thread, NULL);
        if (ret)
                {
                errno = ret;
                printf ("Failed to create dumping thread.\nReason: %s.\n", strerror (errno));
                goto error_thread_create;
                }

        return SUCCESS;

        // Bottom section >>>

        error_thread_create:
                logger_initialized = 0;

        error_cond_init:
                close(logger.file);

        error_open_file:
                ring_buffer_destruct(&logger.ring_buffer);

        error_ring_buffer_construct:
                logger_constructed = 0;

        return ERROR;
        }

ret_code_t log_deinit()
        {
        #ifndef LOG_ON
        return SUCCESS;
        #endif

        if (!__sync_bool_compare_and_swap(&logger_constructed, 1, 0))
                {
                // logger is already deinitialized
                fprintf(stderr, "logger: failed to deinit logger. Reason: logger is already deinitialized\n");
                return ERROR;
                }

        if (logger_initialized == 0)
                {
                fprintf(stderr, "logger: failed to deinit logger. Reason: logger is constructed, but not initialized\n");
                return ERROR;
                }

        ret_code_t ret_code = SUCCESS;

        int ret = 0;

        // Sending signal to thread to dump logs for the last time
        pthread_cond_signal(&logger.dumping_cond);

        void * ret_val = NULL;
        ret = pthread_join(logger.dumping_thread, &ret_val);
        if (ret) {
                errno = ret;
                fprintf(stderr,"Failed to join thread.\nReason: %s.\n", strerror (errno));
                ret_code = ERROR;
                }

        ret = close(logger.file);
        if (ret == -1)
                {
                fprintf(stderr, "logger: failed to close logfile. Reason: %s\n", strerror(errno));
                ret_code = ERROR;
                }

        ret = ring_buffer_destruct(&logger.ring_buffer);
        if (ret)
                {
                fprintf(stderr, "logger: failed to destruct ring buffer");
                ret_code = ERROR;
                }

        logger.log_filter_level = LOG_ERROR;

        logger.ring_buffer = NULL;

        logger.file = -1;

        logger_initialized = 0;

        return ret_code;
        }

ret_code_t log_write(log_level_t log_level, char * format, ...)
        {
        #ifndef LOG_ON
        return SUCCESS;
        #endif

        if (!format)
                {
                return WRONG_ARGUMENTS;
                }

        if (log_level < logger.log_filter_level)
                {
                return ERROR;
                }

        log_line_t new_log_line;

        new_log_line.log_level = log_level;

        va_list va_ptr;
        va_start(va_ptr, format);

        char tmp_buffer[LOG_MAX_MSG_LEN + 1];
        memset(tmp_buffer, 0, LOG_MAX_MSG_LEN + 1);

        vsprintf(tmp_buffer, format, va_ptr);
        strcpy(new_log_line.message, tmp_buffer);

        va_end(va_ptr);

        if (log_level == LOG_FATAL)
                {
                char msg_buffer[LOG_MAX_MSG_LEN + LOG_MAX_MSG_PREFIX_LEN + 1];
                sprintf(msg_buffer, "[LOG_FATAL]: %s\n", tmp_buffer);
                write(logger.file, msg_buffer, strlen(msg_buffer));

                void * buffer_backtrace[LOG_MAX_BACKTRACE_LEN];
                const int calls = backtrace(buffer_backtrace, sizeof(buffer_backtrace) / sizeof(void *));
                backtrace_symbols_fd(buffer_backtrace, calls, logger.file);

                return SUCCESS;
                }


        double load = 0;

        ring_buffer_put(logger.ring_buffer, &new_log_line, &load);

        if (load >= LOG_DUMP_LOAD)
                {
                pthread_cond_signal(&logger.dumping_cond);
                }

        return SUCCESS;
        }

static void * _logger_dumping_thread(void * arg)
        {
        #ifndef LOG_ON
        return NULL;
        #endif

        // Allocating buffer for getting log lines from ring buffer
        log_line_t * buffer[LOGGER_BUFFER_SIZE];

        int i = 0;
        for (i = 0; i < LOGGER_BUFFER_SIZE; i++)
                {
                buffer[i] = (log_line_t *)calloc(1, sizeof(log_line_t));
                if (!buffer[i])
                        {
                        fprintf(stderr, "logger: failed to allocate memory for buffer in dumping thread function. Reason: %s\n", strerror(errno));
                        goto error_buffer_mem_alloc;
                        }
                }

        char * buffer_to_dump = (char *)calloc(LOGGER_BUFFER_SIZE * (LOG_MAX_MSG_LEN + LOG_MAX_MSG_LEN + 1), sizeof(char));
        if (!buffer_to_dump)
                {
                fprintf(stderr, "logger: failed to allocate memory for buffer in dumping thread function. Reason: %s\n", strerror(errno));
                goto error_buffer_to_dump_mem_alloc;
                }
        memset(buffer_to_dump, 0, LOGGER_BUFFER_SIZE * (LOG_MAX_MSG_LEN + LOG_MAX_MSG_LEN + 1));

        uint32_t read_n  = 0;

        while(!__sync_bool_compare_and_swap(&logger_constructed, 0, 0)) // atomic compare
                {
                pthread_cond_wait(&logger.dumping_cond, &tmp_mutex);

                read_n  = 0;
                ring_buffer_getn(logger.ring_buffer, (ring_buffer_elem_t **)&buffer, LOGGER_BUFFER_SIZE, &read_n);

                _dump_to_file(buffer, buffer_to_dump, read_n);

                memset(buffer_to_dump, 0, LOGGER_BUFFER_SIZE * (LOG_MAX_MSG_LEN + LOG_MAX_MSG_LEN + 1));
                }

        // Reading from ring buffer for the last time after deinitialization

        read_n  = 0;
        ring_buffer_getn(logger.ring_buffer, (ring_buffer_elem_t **)&buffer, LOGGER_BUFFER_SIZE, &read_n);

        _dump_to_file(buffer, buffer_to_dump, read_n);

        free(buffer_to_dump);

        // Bottom section >>>
        error_buffer_to_dump_mem_alloc:

        error_buffer_mem_alloc:
                for (i = 0; i < LOGGER_BUFFER_SIZE; i++)
                        {
                        free(buffer[i]);
                        }

        return NULL;
        }


ret_code_t _dump_to_file(log_line_t ** buffer, char * buffer_to_dump, uint32_t n_lines)
        {
        #ifndef LOG_ON
        return SUCCESS;
        #endif

        size_t byte_num = 0;
        char tmp_buffer[LOG_MAX_MSG_LEN + LOG_MAX_MSG_PREFIX_LEN + 1];

        int i = 0;
        for (i = 0; i < n_lines; i++)
                {
                switch (buffer[i]->log_level)
                        {
                        case LOG_DEBUG:
                                sprintf(tmp_buffer, "[DEBUG]: %s\n", buffer[i]->message);
                                break;
                        case LOG_INFO:
                                sprintf(tmp_buffer, "[LOG_INFO]: %s\n", buffer[i]->message);
                                break;
                        case LOG_WARNING:
                                sprintf(tmp_buffer, "[LOG_WARNING]: %s\n", buffer[i]->message);
                                break;
                        case LOG_ERROR:
                                sprintf(tmp_buffer, "[LOG_ERROR]: %s\n", buffer[i]->message);
                                break;
                        case LOG_FATAL:
                                sprintf(tmp_buffer, "[LOG_FATAL]: %s\n", buffer[i]->message);
                                break;
                        default:
                                sprintf(tmp_buffer, "[UNKNOWN]: %s\n", buffer[i]->message);
                                break;
                        }

                strcat(buffer_to_dump, tmp_buffer);

                byte_num += strlen(tmp_buffer);
                }

        write(logger.file, buffer_to_dump, byte_num);

        return SUCCESS;
        }