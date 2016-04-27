#ifndef __CHUNK_MANAGER_H_
#define __CHUNK_MANAHER_H_

#include "chunk.h"
#include "../rbtree/rbtree.h"

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

#define POOL_SIZE 1024
#define MAX_CHUNK_SIZE 4096*4096
#define MIN_CHUNK_SIZE 1024*4096

struct chunk_manager {
	int fd;
	int prot;
 	struct chunk chunk_pool[POOL_SIZE];
	struct rbtree_t *rbtree;

	int cur_chunk_index;
};

int chunk_manager_init (struct chunk_manager *cm, int fd, int mode);
int chunk_manager_finalize (struct chunk_manager *cm);

long int chunk_manager_offset2chunk (struct chunk_manager *cm, long int offset, long int length, struct chunk ** ret_ch, int *chunk_offset, int remap);

#endif
