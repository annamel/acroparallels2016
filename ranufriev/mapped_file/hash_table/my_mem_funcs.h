#ifndef __MY_MEM_FUNCS_H_INCLUDED
#define __MY_MEM_FUNCS_H_INCLUDED

#include <stdlib.h> // for rand
#include <malloc.h>
#include <string.h> // for memcpy


//==============================================================================
#define PROBAB_NUMB 20
extern unsigned seed;
//==============================================================================
void* my_calloc (size_t nmemb, size_t size)
        {
        if ((rand() % PROBAB_NUMB) == 0)
                return (calloc (nmemb, size));
        else
                return NULL;
        };

void* my_memcpy (void *dest, const void *src, size_t n)
        {
        if ((rand() % PROBAB_NUMB) == 0)
                return (memcpy (dest, src, n));
        else
                return NULL;
        };
//==============================================================================


#endif // __MY_MEM_FUNCS_H_INCLUDED

