#ifndef __ERROR_H_INCLUDED
#define __ERROR_H_INCLUDED


//==============================================================================
// '__thread' - storage class keyword, that means that this variable has
//      thread-local storage (TLS), for more info check:
//      https://gcc.gnu.org/onlinedocs/gcc-3.3/gcc/Thread-Local.html
extern __thread int my_errno;

typedef enum errno_error_codes
        {
        no_error = 0,

        mem_err,

        mutex_err,
        mutex_init_err,
        mutex_destr_err,

        pth_create_failed,
        pth_join_failed,

        wrong_args,

        ring_buf_empty,
        ring_buf_fail,

        open_failed,
        close_failed,
        /*
        wrong_hash_func,

        table_empty,
        entry_not_found,

        */
        numb_of_error_codes

        } error_code_t;
//==============================================================================

//==============================================================================
// exactly the same as 'perror' func: i.e. if usr_str = "my_func",
//      and my_errno = no_error, then following will be printed to stdout
//
// $ my_func: no error has occurred
//
void my_perror (const char* usr_str);
//==============================================================================


#endif // __ERROR_H_INCLUDED

