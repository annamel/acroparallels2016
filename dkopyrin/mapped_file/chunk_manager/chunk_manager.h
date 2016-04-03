#ifndef __CHUNK_MANAGER_H_
#define __CHUNK_MANAHER_H_

#include "chunk.h"
#include "../hash/hash_table.h"
#define POOL_SIZE 1

struct chunk_manager {
	int fd;
	int prot;
 	struct chunk chunk_pool[POOL_SIZE];
	struct hashtable ht;
};

int chunk_manager_init (struct chunk_manager *cm, int fd, int mode);
int chunk_manager_finalize (struct chunk_manager *cm);

long int chunk_manager_offset2chunk (struct chunk_manager *cm, long int offset, long int length, struct chunk ** ret_ch, int *chunk_offset);

#endif
