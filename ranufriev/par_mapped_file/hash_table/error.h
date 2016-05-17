#ifndef __ERROR_H_INCLUDED
#define __ERROR_H_INCLUDED


//==============================================================================
extern int my_errno;

typedef enum errno_error_codes
        {
        no_error = 0,

        mem_err,

        wrong_args,
        wrong_hash_func,

        table_empty,
        entry_not_found,

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

