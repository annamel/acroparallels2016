#ifndef __CHUNK_MANAGER_H_
#define __CHUNK_MANAGER_H_

#include "chunk.h"
#include "../nbds/include/skiplist.h"

#include <pthread.h>

#define MIN_CHUNK_SIZE 128LL*1024LL*1024LL
#define CHUNK_MASK ~(MIN_CHUNK_SIZE-1)
//Don't POOL_SIZE more than 65536 - chunks with too big index won't be used
#define POOL_SIZE 1024
#define POOL_SIZE_MASK ~(POOL_SIZE-1)

struct chunk_manager {
	int fd;
	//pthread_mutex_t pool_lock;
       skiplist_t *skiplist;
	struct chunk chunk_pool[POOL_SIZE];
       unsigned short cur_chunk_index;
};

int chunk_manager_init (struct chunk_manager *cm, int fd, int mode);
int chunk_manager_finalize (struct chunk_manager *cm);

long int chunk_manager_gen_chunk (struct chunk_manager *cm, off_t offset, size_t length, struct chunk ** ret_ch, off_t *chunk_offset);
long int chunk_manager_force_gen_chunk (struct chunk_manager *cm, off_t offset, size_t length, struct chunk ** ret_ch, off_t *chunk_offset);

#endif
