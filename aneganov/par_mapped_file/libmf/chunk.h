#ifndef __MF_CHUNK_H__
#define __MF_CHUNK_H__

#include <stdbool.h>
#include "map.h"
#include "list.h"

typedef struct list_head list_t;
typedef struct chunk_pool chpool_t;

typedef struct chunk {
	chpool_t *cpool;
	hkey_t key;
	bool is_indexed;
	unsigned ref_cnt;
	list_t list;
	void *payload;
} chunk_t;

#endif