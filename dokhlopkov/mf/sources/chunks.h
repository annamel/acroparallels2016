#ifndef __MF_CHUNKS__
#define __MF_CHUNKS__

#include <errno.h>

typedef struct Chunk chunk_t;
typedef struct ChunkPool cpool_t;

int chunk_acquire(cpool_t *cpool, off_t offset, size_t size, chunk_t **chunk);
int chunk_get_mem(chunk_t *chunk, off_t offset, void **buf);
int chunk_free(chunk_t);
int chunk_find(cpool_t *cpool, off_t offset, size_t size, chunk_t **chunk);

int cpool_construct(size_t max_mem, int fd, int prot, cpool_t **cpool);
int cpool_destruct(cpool_t **cpool);
int cpool_mem_add(void *ptr, chunk_t *chunk);
int cpool_mem_get(cpool_t *cpool, void *ptr, chunk_t **chunk);
int cpool_fd(cpool_t *cpool);

#endif
