#ifndef __CHUNK_MANAGER_H_
#define __CHUNK_MANAGER_H_

#include "../chunk_queue/chunk_queue.h"
#include "../nbds/include/skiplist.h"

#include <pthread.h>

#define MIN_CHUNK_SIZE 128LL*1024LL*1024LL
#define CHUNK_MASK ~(MIN_CHUNK_SIZE-1)

struct chunk_manager {
	int fd;
	struct chunk_queue queue;
	pthread_mutex_t pool_lock;
       skiplist_t *skiplist;
       unsigned cur_chunk_index: LOG_POOL_SIZE;
};

int chunk_manager_init (struct chunk_manager *cm, int fd, int mode);
int chunk_manager_finalize (struct chunk_manager *cm);

long int chunk_manager_gen_chunk (struct chunk_manager *cm, off_t offset, size_t length, struct chunk ** ret_ch, off_t *chunk_offset);

#endif
