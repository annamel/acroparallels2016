#ifndef __MALLOCS_H__
#define __MALLOCS_H__

#include <errno.h>

int _malloc(size_t size, void **ptr);
int _calloc(size_t size, void **ptr);
int _realloc(size_t size, void **ptr);
int _free(void **ptr);

#endif
