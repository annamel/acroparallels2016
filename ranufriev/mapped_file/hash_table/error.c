#include "error.h"
#include <stdio.h>

//==============================================================================
// Declaring of structures
//==============================================================================
int my_errno = 0;

static const char* errno_error_definit[numb_of_error_codes + 1] =
        {
        [no_error]        = "no error has occurred",

        [mem_err]         = "memory error has occured",

        [wrong_args]      = "wrong arguments was given to the function",
        [wrong_hash_func] = "hash function gives a key, which value is > than \
                                        size of hash_table",

        [table_empty]     = "hash table is currently empty",
        [entry_not_found] = "entry with key that you specified is not found",
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
                printf ("%s: %s\n", usr_str, errno_error_definit[my_errno]);
                return;
                }

// if smth is wrong
printf ("wrong args were given to 'my_perror' func\n");
}
//==============================================================================

