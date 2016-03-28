#include "chunk_manager.h"
#include "../chunk/chunk.h"
#include "../logger/log.h"

int chunk_manager_init (struct chunk_manager *cm){
	cm -> chunk_pool = malloc(POOL_SIZE * sizeof(struct chunk));
	if (!cm -> chunk_pool) {

	}
}

int chunk_manager_finalize (struct chunk_manager *cm){
	free(cm -> chunk_pool);
}
