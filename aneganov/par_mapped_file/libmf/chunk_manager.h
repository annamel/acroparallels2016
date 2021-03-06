#ifndef __MF_CHUNK_MANAGER__
#define __MF_CHUNK_MANAGER__

#include <sys/types.h>
#include "mfdef.h"

typedef struct chunk chunk_t;
typedef struct chunk_pool chpool_t;

__must_check int chunk_acquire(chpool_t *cpool, off_t offset, size_t size, chunk_t **chunk);
__must_check int chunk_find(chpool_t *cpool, off_t offset, size_t size, chunk_t **chunk);
__must_check int chunk_get_mem(chunk_t *chunk, off_t offset, void **buf, off_t *border);
__must_check int chpool_construct(int fd, int prot, chpool_t **cpool);
__must_check int chpool_destruct(chpool_t *cpool);
__must_check int chpool_fd(chpool_t *cpool);

int chunk_release(chunk_t *chunk);

off_t  chpool_fsize(chpool_t *cpool);
size_t chpool_page_size(chpool_t *cpool);

#endif