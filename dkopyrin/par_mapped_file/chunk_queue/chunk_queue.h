#ifndef __CHUNK_QUEUE_H_
#define __CHUNK_QUEUE_H_

#include "chunk.h"
#define POOL_SIZE 1024
#define LOG_POOL_SIZE 10

struct chunk_queue{
	int size;
	volatile struct chunk *head;
	volatile struct chunk *tail;

	struct chunk chunk_pool[POOL_SIZE];
};

int chunk_queue_init(struct chunk_queue * cq);
void chunk_queue_finalize(struct chunk_queue * cq);

#endif
