#ifndef chunk_manager_h
#define chunk_manager_h

#include "common_types.h"
#include "sys/mman.h"
#include "list.h"
#include "hash_table.h"

int ch_init(off_t index, off_t length, ch_pool_t *ch_pool);
ch_pool_t *ch_pool_init(int fd, int prot);
int ch_pool_deinit(ch_pool_t* ch_pool);
int ch_find(ch_pool_t *ch_pool, off_t index, off_t length);
int chunk_release(chunk_t* chunk);
int chunk_clear(chunk_t* chunk);
size_t get_chunk_size(off_t multiplier);

#endif  
