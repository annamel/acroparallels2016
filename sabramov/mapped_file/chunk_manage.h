#ifndef CHUNK_MANAGE
#define CHUNK_MANAGE

#include "hash_table.h"
#include "mapped_file.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAX_MEM_USAGE	0x0400000 


typedef struct chunk 
{
	size_t chunk_size;
	void* chunk_addr;
	struct chunk_pool* pool;
	int ref_count;
	struct chunk* next;
	struct chunk* prev;	
	int is_in_pool;
	int pg_multiple_offset;
} chunk_t;

typedef struct chunk_pool
{
	int fd;
	struct hash_node** hash_table;
	struct chunk* mru_chunk;
	int max_mem_usage;
	int current_mem_usage;
} chunk_pool_t;


#endif  // CHUNK_MANAGE