#ifndef CHUNK_MANAGE
#define CHUNK_MANAGE

#define INIT_CHUNKS_NUMB 50
#define INIT_POOL_LENGTH 10
#define MAX_MEM_USAGE	0x0400000          // FIXME: not sure that it is necessary 

#include <sys/types.h>

typedef struct chunk
{
	off_t pg_multiple_offset;
	size_t size;
	void* addr;
	int ref_count;
	struct chunk* next;
	struct chunk* prev;
	struct chunk* next_iter;
	struct chunk* prev_iter;
} chunk_t;

typedef struct file_handle
{
	int fd;
	struct chunk** pool;
	struct chunk* free_chunks;
	struct chunk* free_chunks_tail;	
	struct chunk** hash_table;
	int cur_subpool_numb; 	
	int subpools_numb;		 
	int max_mem_usage;
	int cur_mem_usage;
} file_handle_t;






#endif	// CHUNK_MANAGE