#include "error.h"
#include <stdio.h>

//==============================================================================
// Declaring of structures
//==============================================================================
__thread int my_errno = 0;

static const char* errno_error_definitions[numb_of_error_codes/* + 1*/] =
        {
        [no_error]          = "no error has occurred",

        [mem_err]           = "memory error has occurred",

        [mutex_err]         = "mutex error has occurred",
        [mutex_init_err]    = "mutex initialization has failed",
        [mutex_destr_err]   = "mutex destruction has failed",

        [pth_create_failed] = "pthread_create() has failed",
        [pth_join_failed]   = "pthread_join() has failed",

        [wrong_args]        = "wrong arguments was given to the function",

        [ring_buf_empty]    = "ring buffer is currently empty",
        [ring_buf_fail]     = "func form ring buffer lib has failed",

        [open_failed]       = "open() has failed",
        [close_failed]      = "close() has failed",
        };
//==============================================================================

//==============================================================================
// Realisation of functions
//==============================================================================

// exactly the same as 'perror' func (for example look at 'error.h' file)
inline void my_perror (const char* usr_str)
{
if (usr_str != NULL)
        if (*usr_str != '\0')
                {
                printf ("%s: %s\n", usr_str, errno_error_definitions[my_errno]);
                return;
                }

// if smth is wrong
printf ("wrong args were given to 'my_perror' func\n");
}
//==============================================================================

