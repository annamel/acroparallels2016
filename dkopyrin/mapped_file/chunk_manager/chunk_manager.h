#ifndef __CHUNK_MANAGER_H_
#define __CHUNK_MANAGER_H_

#include "chunk.h"
#include "../rbtree/rbtree.h"

#define POOL_SIZE 2048
#define LOG_POOL_SIZE 11
#define DEFAULT_CHUNK_SIZE 64LL*1024LL*1024LL
#define MIN_CHUNK_SIZE 4096LL
#define MAX_CHUNK_SIZE 1024LL*1024LL*1024LL

#define MASK(size) (~(size-1))

struct chunk_manager {
	size_t cur_chunk_size;
	int fd;
	struct chunk chunk_pool[POOL_SIZE];
	struct rbtree_t *rbtree;
	unsigned cur_chunk_index: LOG_POOL_SIZE;
};

int chunk_manager_init (struct chunk_manager *cm, int fd, int mode);
int chunk_manager_finalize (struct chunk_manager *cm);

ssize_t chunk_manager_gen_chunk (struct chunk_manager *cm, off_t offset, size_t length, struct chunk ** ret_ch, off_t *chunk_offset);

#endif
