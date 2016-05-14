#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG

#include <stdio.h>

#define DEBUG_LOG(format) fprintf(stderr,"%s:%d:%s -> hash_table: " format "\n", __FILE__, __LINE__, __func__)

#define DEBUG_LOG_ARGS(format, ...) fprintf(stderr,"%s:%d:%s -> hash_table: " format "\n", __FILE__, __LINE__, __func__, __VA_ARGS__)

#else

#define DEBUG_LOG(...)
#define DEBUG_LOG_ARGS(...)

#endif // DEBUG

#endif // DEBUG_H