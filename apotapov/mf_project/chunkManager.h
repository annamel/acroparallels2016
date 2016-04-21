#ifndef chunk_manager
#define chunk_manager

#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <assert.h>
#include "malloc.h"
#include "hash.h"
//#include "logger.h"
#include <errno.h>

typedef struct chunk chunk_t;
typedef struct chunk_pool ch_pool_t;

struct chunk {
	ch_pool_t *ch_pool;
	off_t index;
	off_t length;
	unsigned int refer_counter;
	chunk_t *next;
	chunk_t *prev;
	void *pld; 
};

struct chunk_pool {
	int file_d;
	int prot;
	size_t size;
	size_t num_of_pages;
	chunk_t *ch_list;
    hash_table_t *hashtable;
    hash_table_t *mem_hash_table;
};

static size_t get_ch_size (off_t multiplier);
static int ch_init(ch_pool_t *ch_pool, chunk_t *chunk, off_t index, off_t length);
static int ch_destr(chunk_t *chunk);
static int ch_create(ch_pool_t *ch_pool, chunk_t *chunk, off_t index, off_t length);
static int ch_get(ch_pool_t *ch_pool, chunk_t *chunk, off_t index, off_t length);
int ch_acquire(ch_pool_t *ch_pool, chunk_t *chunk, off_t offset, size_t size);
static int ch_pool_del(chunk_t *chunk);
static int ch_pool_add(chunk_t *chunk);
int ch_release(chunk_t *chunk);
static int ch_pool_init(ch_pool_t *ch_pool, unsigned size, int fd, int prot);
int ch_pool_construct(ch_pool_t *ch_pool, size_t max_m, int file_d, int prot);
int ch_pool_destruct(ch_pool_t *ch_pool);
int ch_get_mem(chunk_t *chunk, off_t offset, void *buf);
int ch_pool_fd(ch_pool_t *ch_pool);
int ch_pool_mem_add(void *ptr, chunk_t *chunk);
int ch_pool_mem_get(ch_pool_t *ch_pool, void *ptr, chunk_t *chunk);
int chunk_find(ch_pool_t *cpool, off_t offset, size_t size, chunk_t *chunk);

#endif
