#ifndef chunk_manager_h
#define chunk_manager_h

#include "sys/mman.h"
#include "list.h"
#include "hash_table.h"

#define DEFAULT_HASH_TABLE_SIZE 1024
#define DEFAULT_ARRAY_SIZE 1024

typedef struct chunk chunk_t;
typedef struct ch_pool ch_pool_t;

struct chunk {
    off_t index;
    off_t length;
    unsigned int ref_counter;
    void *data;
    ch_pool_t *ch_pool;
};

struct ch_pool {
    int fd;
    int prot;
    size_t arrays_cnt;
    chunk_t **pool;
    hash_table_t *h_table;
    list_t *list_zero_ref_count;
    list_t *list_of_free_chunks;
};

int ch_init(off_t index, off_t length, ch_pool_t *ch_pool);
ch_pool_t *ch_pool_init(int fd, int prot);
int ch_pool_deinit(ch_pool_t* ch_pool);
int ch_find(ch_pool_t *ch_pool, off_t index, off_t length);
int chunk_release(chunk_t* chunk);
int chunk_clear(chunk_t* chunk);
size_t get_chunk_size(off_t multiplier);
#endif 