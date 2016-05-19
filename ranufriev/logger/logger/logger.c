#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <execinfo.h>

#include <assert.h>

#include "../ring_buf/ring_buf.h"
#include "logger.h"

#if (IS_LOGGER_ENABLED == 1)


#define RING_BUF_SIZE 128
//==============================================================================
// Variables and structures
//------------------------------------------------------------------------------
typedef struct Logger
        {
        int is_constructed;

        log_level_t min_log_level;
        int fd;

        ring_buf_ptr ring_buf;

        pthread_t       dumping_thread;
        pthread_cond_t  dumping_cond;
        pthread_mutex_t dumping_mtx; // not used really, only for 'dumping_cond'
        } logger_t;

//==============================================================================

//==============================================================================
// Prototypes of internal functions
//------------------------------------------------------------------------------
static void* _Logger_dumping_func (void* );
//==============================================================================
//==============================================================================
// Realisation of functions
//------------------------------------------------------------------------------
logger_t logger = {0};

int Logger_construct (log_level_t min_log_level, const char* log_file_path)
{
if (log_file_path == NULL)
        {
        my_errno = wrong_args;
        return -1;
        }

if (__sync_bool_compare_and_swap (&(logger.is_constructed), 0, 1) == 0)
        {
        printf ("Logger has already been constructed!\n");
        return -1;
        }

logger.min_log_level = min_log_level;

if ((logger.fd = open (log_file_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) == -1)
        {
        my_errno = open_failed;
        goto error_open;
        }

if (Ring_buf_construct (&(logger.ring_buf), RING_BUF_SIZE) == -1)
        goto error_ring_buf_constr;

if (pthread_cond_init (&(logger.dumping_cond), NULL) == -1)
        {
        my_errno = mutex_err;
        goto error_pth_cond_init;
        }

if (pthread_mutex_init (&(logger.dumping_mtx), NULL) == -1)
        {
        my_errno = mutex_err;
        goto error_pth_mtx_init;
        }

if (pthread_create (&(logger.dumping_thread), NULL, _Logger_dumping_func, NULL))
        {
        my_errno = pth_create_failed;
        goto error_pth_create;
        }

return 0;


error_pth_create:
        assert (pthread_mutex_destroy (&(logger.dumping_mtx)) == 0);
error_pth_mtx_init:
        assert (pthread_cond_destroy (&(logger.dumping_cond)) == 0);
error_pth_cond_init:
        Ring_buf_destruct (&(logger.ring_buf));
error_ring_buf_constr:
        assert (close (logger.fd) == 0);
error_open:
        logger.is_constructed = 0;
        return -1;
}


int Logger_destruct ()
{
if (__sync_bool_compare_and_swap (&(logger.is_constructed), 1, 0) == 0)
        {
        printf ("Logger has already been destructed!\n");
        return -1;
        }

pthread_cond_signal (&(logger.dumping_cond));

if (pthread_join (logger.dumping_thread, NULL) != 0)
        {
        my_errno = pth_join_failed;
        return -1;
        }

if (pthread_mutex_destroy (&(logger.dumping_mtx)) != 0)
        {
        my_errno = mutex_destr_err;
        return -1;
        }

if (pthread_cond_destroy (&(logger.dumping_cond)) != 0)
        {
        my_errno = mutex_err;
        return -1;
        }

Ring_buf_destruct (&(logger.ring_buf));

if (close (logger.fd) != 0)
        {
        my_errno = close_failed;
        return -1;
        }

return 0;
}


int Logger_log (log_level_t log_level, const char* format_str, ...)
{
va_list argp;
char tmp_str[MAX_LENGTH_OF_LOG_STRING + 1] = {0};
int numb_of_wrtn_bytes = 0;

void* bctrace_arr[MAX_LENGTH_OF_BACKTRACE] = {0};
int bctrace_size = 0;

int cur_occupancy = 0;

if (format_str == NULL)
        {
        my_errno = wrong_args;
        return -1;
        }

if (log_level < logger.min_log_level)
        return 0;

switch (log_level)
        {
        case LOG_DEBUG:
                numb_of_wrtn_bytes = sprintf (tmp_str, "[DEBUG]: ");
                break;
        case LOG_INFO:
                numb_of_wrtn_bytes = sprintf (tmp_str, "[LOG_INFO]: ");
                break;
        case LOG_WARNING:
                numb_of_wrtn_bytes = sprintf (tmp_str, "[LOG_WARNING]: ");
                break;
        case LOG_ERROR:
                numb_of_wrtn_bytes = sprintf (tmp_str, "[LOG_ERROR]: ");
                break;
        case LOG_FATAL:
                numb_of_wrtn_bytes = sprintf (tmp_str, "[LOG_FATAL]: ");
                break;

        default:
                assert (!"Can not be here!");
        }

va_start (argp, format_str);

numb_of_wrtn_bytes += vsnprintf (tmp_str + numb_of_wrtn_bytes, MAX_LENGTH_OF_LOG_STRING - numb_of_wrtn_bytes, format_str, argp);
tmp_str[numb_of_wrtn_bytes + 1] = '\0';
// printf ("%s\n", tmp_str);

va_end (argp);

if (log_level == LOG_FATAL)
        {
        write (logger.fd, tmp_str, strlen (tmp_str));

        bctrace_size = backtrace (bctrace_arr, MAX_LENGTH_OF_BACKTRACE);
        backtrace_symbols_fd (bctrace_arr, bctrace_size, logger.fd);

        return 0;
        }

if ((cur_occupancy = Ring_buf_put (logger.ring_buf, &tmp_str, numb_of_wrtn_bytes + 1)) == -1)
        {
        my_errno = ring_buf_fail;
        return -1;
        }

if (cur_occupancy >= MAX_BUF_OCCUPANCY)
        {
        pthread_cond_signal (&(logger.dumping_cond));
        // printf ("signal!\n");
        }

return 0;
}


void* _Logger_dumping_func (void* param)
{
size_t i = 0;
ring_buf_N_elems_ptr buf_ptr = NULL;

while (!__sync_bool_compare_and_swap (&(logger.is_constructed), 0, 0))
        {
        pthread_cond_wait (&(logger.dumping_cond), &(logger.dumping_mtx));

        if (Ring_buf_get_N_elems (logger.ring_buf, &buf_ptr, RING_BUF_SIZE) != 0)
                continue;

        for (i = 0; i < buf_ptr->numb_of_elems; i++)
                write (logger.fd, buf_ptr->buf_of_elems[i].buf_for_entry, buf_ptr->buf_of_elems[i].size_of_entry);

        Ring_buf_delete_elems_after_get_N (&buf_ptr);
        }

if (Ring_buf_get_N_elems (logger.ring_buf, &buf_ptr, RING_BUF_SIZE) == 0)
        {
        for (i = 0; i < buf_ptr->numb_of_elems; i++)
                write (logger.fd, buf_ptr->buf_of_elems[i].buf_for_entry, buf_ptr->buf_of_elems[i].size_of_entry);

        Ring_buf_delete_elems_after_get_N (&buf_ptr);
        }

return NULL;
}



//==============================================================================
#else
int Logger_construct (log_level_t min_log_level, const char* log_file_path) { return 0; }
int Logger_destruct  () { return 0; }

int Logger_log (log_level_t log_level, const char* format_str, ...) { return 0; }
#endif
