#ifndef __CHUNK_MANAGER_H_
#define __CHUNK_MANAHER_H_

#include "../chunk/chunk.h"
#define POOL_SIZE 1

struct chunk_manager {
	struct chunk * chunk_pool;
}

int chunk_manager_init (struct chunk_manager *cm);
int chunk_manager_finalize (struct chunk_manager *cm);

#endif
