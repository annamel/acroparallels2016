#ifndef __MALLOCS_H__
#define __MALLOCS_H__

#include <errno.h>

#define MF_MALLOC_POISON 0xBEEFDEAD

int mf_malloc(size_t size, void **ptr);
int mf_calloc(size_t size, void **ptr);
int mf_realloc(size_t size, void **ptr);
int mf_free(size_t size, void **ptr);

#endif
