#ifndef __MF_CHUNKS__
#define __MF_CHUNKS__

#include <errno.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include "hashtable/hashtable.h"
#include "mallocs.h"

typedef struct ChunkPool cpool_t;
typedef struct Chunk chunk_t;

struct ChunkPool {
	int fd;
	size_t threshold;
	size_t nr_pages;
	chunk_t *head;
	hashtable_t *ht;
	sem_t sem;
};

struct Chunk {
	cpool_t *cpool;
	off_t idx;
	off_t len;
	unsigned ref_cnt;
	chunk_t *next;
	chunk_t *prev;
	void *payload;
};

int ch_acquire(cpool_t *cpool, off_t offset, size_t size, chunk_t **chunk);
int ch_get_mem(chunk_t *chunk, off_t offset, void **buf);
int ch_release(chunk_t *chunk);
int ch_find(cpool_t *cpool, off_t offset, size_t size, chunk_t **chunk);

int cpool_construct(size_t max_mem, int fd, cpool_t **cpool);
int cpool_destruct(cpool_t *cpool);
int cpool_mem_add(void *ptr, chunk_t *chunk);
int cpool_mem_get(cpool_t *cpool, void *ptr, chunk_t **chunk);
int cpool_fd(cpool_t *cpool);

#endif

