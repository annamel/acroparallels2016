#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#define DEFAULT_HASH_TABLE_SIZE 1024
#define DEFAULT_ARRAY_SIZE 1024

#include <sys/mman.h>
#include "hash_table/hash_table.h"
#include "List/list.h"

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

typedef struct ChunkPool chunk_pool_t;
typedef struct Chunk chunk_t;

struct ChunkPool {
    int fd;
    int protection;
    list_t* free_chunks;
    list_t* zero_chunks;
    chunk_t** loafs;
    unsigned loafs_count;
    htable_t* hash;
};

struct Chunk {
    off_t index;
    off_t len;
    unsigned ref_counter;
    void* data;
    chunk_pool_t* cpool;
};

chunk_t* chunk_init(off_t index, off_t len, chunk_pool_t* cpool);
chunk_pool_t* chunk_pool_init(int fd, int prot);
int chunk_deinit(chunk_t* chunk);
int chunk_pool_deinit(chunk_pool_t* cpool);
int chunk_pool_find(chunk_pool_t* cpool, chunk_t** chunk, off_t index, off_t len);
int get_chunk_size(off_t len);
int chunk_release(chunk_t* chunk);

#endif // CHUNKMANAGER_H
