#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <errno.h>
#include <sys/mman.h>
#include <malloc.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#include "hash_table/hash_table.h"
//#include "logger/logger.h"



#define DEFAULT_CHPOOL_SIZE 1024


typedef struct chunk chunk_t;
typedef struct chunk_pool chpool_t;


struct chunk
{
    chpool_t *chpool;
    off_t index;
    off_t len;
    unsigned int ref_cnt;
    chunk_t *next;
    chunk_t *prev;
    void *data;

};



struct chunk_pool
{
    int fd;
    int prot;
    size_t size;
    size_t nr_pages;
    chunk_t *chunk_list;
    hash_table_t *ht;
    hash_table_t *mem_ht;

};



int chunk_acquire(chunk_t *chunk, chpool_t *chpool, off_t offset, size_t size);
int chunk_find(chunk_t *chunk, chpool_t *chpool,size_t size, off_t offset);
int chunk_release(chunk_t *chunk);
int chunk_get_mem(chunk_t *chunk, off_t offset, void *buf);
int chpool_construct(chpool_t *chpool, size_t mem_limit, int fd, int prot);
int chpool_destruct(chpool_t *chpool);
int chpool_fd(chpool_t *chpool);
int chpool_mem_add(void *ptr, chunk_t *chunk);
int chpool_mem_get(chpool_t *chpool, void *ptr, chunk_t *chunk);



#endif // CHUNK_MANAGER_H
