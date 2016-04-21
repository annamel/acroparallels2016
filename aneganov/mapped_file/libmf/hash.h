#ifndef __MF_HASH_H__
#define __MF_HASH_H__

#include <sys/types.h>

typedef off_t hkey_t;
typedef void* hval_t;

#define HASHTABLE_SIZE 1024
inline static size_t simple_hash(hkey_t x) {
	return x % HASHTABLE_SIZE;
}

#endif
