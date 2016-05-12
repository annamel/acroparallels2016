#ifndef CHUNK_MANAGE
#define CHUNK_MANAGE

#include <sys/types.h>

#define INIT_CHUNKS_NUM 100
#define INIT_POOL_LENGTH 10
#define MAX_MAPPED_MEM  0x040000000
#define DEF_PAGE_NUM	4096              						
#define DEF_CHUNK_SIZE  (DEF_PAGE_NUM*4096)
#define POWER			24

#define TABLE_SIZE 997
 
/* 
 * Description of chunk management structure: 
 * There is chunk pool to manage chunks. Pool is
 * represented by array of subpools (struct chunk** pool). 
 * So it is not needed to alloc a lot of chunks when file  
 * is just opened, because library provides quite big size of    
 * chunk by default. If new chunk is required and there is no 
 * free space in current subpool only in this time new subpool 
 * is allocated. In this way lib provides some kind of balance
 * between speed (allocating big number of chunks at a time to 
 * avoid calling allocating functions often) and effective memory
 * usage (do not allocate very lagre number of chunks at a time 
 * because with big probability it is not necessary). One more 
 * feature of lib is free chunk list represented by free_chunks   
 * pointer. In this way it is possible to list all free chunks  
 * because every chunk has such fields as next and prev. 
 * So there is an opportunity to get free chunk if needed and
 * put waste chunk to free list (via ponter free_chunks_tail 
 * that ponts to the end of free list).
 * @uthor: Semyon Abramov, MIPT (semyon.abramov.mipt@gmail.com). 
 */

typedef struct chunk
{
	off_t pg_round_down_off;
	size_t size;
	void* addr;
	long long ref_count;
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
	int cur_subpool_num; 	
	int subpools_num;		 
	long long cur_mem_usage;
	long long file_size;
	long long page_size;
	int is_mapped;
} file_handle_t;


static chunk_t* mapped_file;


#endif	// CHUNK_MANAGE