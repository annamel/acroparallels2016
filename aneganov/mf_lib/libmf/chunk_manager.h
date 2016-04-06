#ifndef __MF_CHUNK_MANAGER__
#define __MF_CHUNK_MANAGER__

#include <errno.h>

typedef struct Chunk chunk_t;
typedef struct ChunkPool chpool_t;

int chunk_acquire(chpool_t *cpool, off_t offset, size_t size, chunk_t **chunk);
int chunk_find(chpool_t *cpool, off_t offset, size_t size, chunk_t **chunk);
int chunk_release(chunk_t *chunk);
int chunk_get_mem(chunk_t *chunk, off_t offset, void **buf);
int chpool_construct(size_t max_mem, int fd, int prot, chpool_t **cpool);
int chpool_destruct(chpool_t **cpool);

#endif