#ifndef __MF_MALLOC_H__
#define __MF_MALLOC_H__

#include <errno.h>

#define MF_MALLOC_POISON 0xDEADBABA

int mf_malloc(size_t size, void **ptr);
int mf_zmalloc(size_t size, void **ptr);
int mf_realloc(size_t size, void **ptr);
int mf_free(size_t size, void *ptr);

#endif