#ifndef __MF_MALLOC_H__
#define __MF_MALLOC_H__

#include <errno.h>

#ifdef DEBUG2
void __mf_malloc_debug_init(void) __attribute__((constructor));
#endif

int mf_malloc(size_t size, void **ptr);
int mf_realloc(size_t size, void **ptr);

#define mf_free(ptr) free(ptr)

#endif