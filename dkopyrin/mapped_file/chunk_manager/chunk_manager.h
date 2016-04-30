#ifndef __CHUNK_MANAGER_H_
#define __CHUNK_MANAGER_H_

#include "chunk.h"
#include "../rbtree/rbtree.h"

#define POOL_SIZE 1024
#define LOG_POOL_SIZE 10
#define MIN_CHUNK_SIZE 128LL*1024LL*1024LL
#define CHUNK_MASK ~(MIN_CHUNK_SIZE-1)

struct chunk_manager {
	int fd;
	struct chunk chunk_pool[POOL_SIZE];
	struct rbtree_t *rbtree;
	int cur_chunk_index: LOG_POOL_SIZE;
};

int chunk_manager_init (struct chunk_manager *cm, int fd, int mode);
int chunk_manager_finalize (struct chunk_manager *cm);

long int chunk_manager_gen_chunk (struct chunk_manager *cm, off_t offset, size_t length, struct chunk ** ret_ch, off_t *chunk_offset);

#endif
