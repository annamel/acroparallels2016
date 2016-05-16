#include "chunk_manage.h"
#include "mapped_file.h"

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>

long long page_round_down(off_t offset)			 
{
	return ((offset >> POWER) << POWER);
}

long long page_round_up(off_t offset, size_t size)
{
	return ((((offset + size) >> POWER) + 1) << POWER);
} 

int chunk_init(chunk_t* chunk, off_t pg_round_down_off, size_t size, void* addr)
{
	chunk->pg_round_down_off = pg_round_down_off;
	chunk->size = size;
	chunk->addr = addr;
	chunk->ref_count = 0;
	return 0;
}

int is_chunk_reusable(chunk_t* chunk, int  chunk_size)		
{
	return (chunk->size >= chunk_size); 
}

void chunk_reuse(chunk_t* chunk, off_t offset, long long pg_round_down_off, void** chunk_addr)
{
	*chunk_addr = chunk->addr + (offset - pg_round_down_off);			
}

int chunk_find(mf_handle_t mf, chunk_t** chunk, void** addr, off_t offset, size_t size)
{
	file_handle_t* fh = mf;

	if (fh->is_mapped)
	{	
		*chunk = (chunk_t*)(intptr_t)table_lookup(0, fh);
		(*chunk)->ref_count += 1;
		*addr = (*chunk)->addr + offset; 
		return 0;
	}
	else
	{
		void* map_addr = mmap(NULL, fh->file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fh->fd, 0);

		if (map_addr == (void*)(-1))	
			fh->is_mapped = 0;
		else
		{	
			fh->is_mapped = 1;
			chunk_init(fh->free_chunks, 0, fh->file_size, map_addr);	
			*chunk = fh->free_chunks;
			fh->free_chunks->ref_count += 1;
			push_chunk(0, &(fh->free_chunks), &fh);
			*addr = (*chunk)->addr + offset;
			return 0;
		}		
	}

	long long pg_round_down_off = page_round_down(offset);
	long long pg_round_up_off = page_round_up(offset, size);			
	long long chunk_size = 0;
	
	if (fh->file_size)
	{
		if (pg_round_up_off > fh->file_size)
			chunk_size = fh->file_size - pg_round_down_off;
		else
		{
			if (pg_round_up_off - pg_round_down_off <= DEF_CHUNK_SIZE)
				chunk_size = DEF_CHUNK_SIZE;	
			else
				chunk_size = pg_round_up_off - pg_round_down_off;
		}
	}
	else
		chunk_size = pg_round_up_off - pg_round_down_off;		


	*chunk = (chunk_t*)(intptr_t)table_lookup(pg_round_down_off, fh);

	if (*chunk)
	{				
		if (is_chunk_reusable(*chunk, chunk_size))
		{
			chunk_reuse(*chunk, offset, pg_round_down_off, addr);
			(*chunk)->ref_count += 1;			
			return 0;
		}
	}		

	if (fh->cur_mem_usage + chunk_size > MAX_MAPPED_MEM)	
		unmap_all_notref(fh);

	void* map_addr = mmap(NULL, chunk_size, PROT_READ | PROT_WRITE, MAP_SHARED, fh->fd, pg_round_down_off);
	
	if (map_addr == (void*)(-1))
		return -1;

	*addr = (uint8_t*)map_addr + (offset - pg_round_down_off);	    		
		
	fh->cur_mem_usage += chunk_size;

	if (!fh->free_chunks)
		chunk_subpool_init(fh);

	chunk_init(fh->free_chunks, pg_round_down_off, chunk_size, map_addr);	
	*chunk = fh->free_chunks;
	fh->free_chunks->ref_count += 1;
	push_chunk(pg_round_down_off, &(fh->free_chunks), &fh);		
	
	return 0;			
}

int unmap_all_notref(file_handle_t* fh)
{
	chunk_t** hash_table = fh->hash_table;
	int i = 0;

	for (i = 0; i < TABLE_SIZE; i++)
	{
		while (hash_table[i])
		{
			munmap(hash_table[i]->addr, hash_table[i]->size);
			fh->cur_mem_usage -= hash_table[i]->size;
			chunk_t* ptr = hash_table[i];	
			hash_table[i] =  hash_table[i]->next;
			ptr->next = fh->free_chunks;	
			ptr->prev = NULL;
			fh->free_chunks = ptr;
		}
	}

	return 0;
}


int chunk_subpool_init(file_handle_t* fh)
{

	if ((fh->cur_subpool_num + 1) == fh->subpools_num)
	{
		chunk_t** new_pool = (chunk_t**)realloc(fh->pool, (fh->subpools_num * 2) * sizeof(chunk_t*));

		if (!new_pool)
			return -1;

		fh->pool = new_pool;
		fh->subpools_num *= 2;
	}
	
	fh->cur_subpool_num += 1;
		
	fh->pool[fh->cur_subpool_num] = (chunk_t*)malloc(sizeof(chunk_t) * INIT_CHUNKS_NUM); 

	if (!fh->pool[fh->cur_subpool_num])
		return -1;
	
	chunk_t* chp = fh->pool[fh->cur_subpool_num];

	chunk_t* prev_chp = fh->pool[fh->cur_subpool_num - 1];

	chp[0].prev = &(prev_chp[INIT_CHUNKS_NUM - 1]);
	chp[0].next = &(chp[1]);
	chp[0].prev_iter = &(prev_chp[INIT_CHUNKS_NUM - 1]);
	chp[0].next_iter = &(chp[1]);

	prev_chp[INIT_CHUNKS_NUM - 1].next_iter = &(chp[0]);

	int i = 0;

	for (i = 1; i < INIT_CHUNKS_NUM - 1; i++)
	{
		chp[i].next = &(chp[i+1]);	
		chp[i].prev = &(chp[i-1]);	
		chp[i].next_iter = &(chp[i+1]);	
		chp[i].prev_iter = &(chp[i-1]);
	}

	chp[INIT_CHUNKS_NUM - 1].next = NULL;	
	chp[INIT_CHUNKS_NUM - 1].prev = &(chp[INIT_CHUNKS_NUM - 2]);
	chp[INIT_CHUNKS_NUM - 1].next_iter = NULL;	
	chp[INIT_CHUNKS_NUM - 1].prev_iter = &(chp[INIT_CHUNKS_NUM - 2]);	

	fh->free_chunks = &(chp[0]);
	fh->free_chunks_tail = &(chp[INIT_CHUNKS_NUM - 1]);
	
	return 0;
}

int file_handle_init(const char* pathname, mf_handle_t mf)
{
	file_handle_t* fh = mf;
	fh->fd = open(pathname, O_RDWR | O_CREAT, S_IRWXU);

	if (fh->fd == -1)
	{	
		close(fh->fd);
		return -1;
	}
	
	fh->hash_table = (chunk_t**)(intptr_t)init_hash_table(); 
	fh->cur_mem_usage = 0;		
	fh->subpools_num = INIT_POOL_LENGTH;
	fh->file_size = lseek(fh->fd, 0, SEEK_END);
	fh->page_size = sysconf(_SC_PAGESIZE);
//	fh->lock_map = PTHREAD_MUTEX_INITIALIZER;
    sem_init((&(fh->lock_map)), 0, 1);	
	
	fh->pool = (chunk_t**)malloc(sizeof(chunk_t*) * INIT_POOL_LENGTH);

	if (!fh->pool)
		return -1;

	fh->cur_subpool_num = 0;
	fh->pool[0] = (chunk_t*)malloc(sizeof(chunk_t) * INIT_CHUNKS_NUM);
	chunk_t* chp = fh->pool[0];
	fh->free_chunks = &(chp[0]);

	chp[0].prev = NULL;
	chp[0].next = &(chp[1]);
	chp[0].prev_iter = NULL;
	chp[0].next_iter = &(chp[1]);

	int i = 0;

	for (i = 1; i < INIT_CHUNKS_NUM - 1; i++)
	{
		chp[i].next = &(chp[i+1]);	
		chp[i].prev = &(chp[i-1]);	
		chp[i].next_iter = &(chp[i+1]);	
		chp[i].prev_iter = &(chp[i-1]);
	}

	chp[INIT_CHUNKS_NUM - 1].next = NULL;	
	chp[INIT_CHUNKS_NUM - 1].prev = &(chp[INIT_CHUNKS_NUM - 2]);
	chp[INIT_CHUNKS_NUM - 1].next_iter = NULL;	
	chp[INIT_CHUNKS_NUM - 1].prev_iter = &(chp[INIT_CHUNKS_NUM - 2]);	
	fh->free_chunks_tail = &(chp[INIT_CHUNKS_NUM - 1]);

	void* map_addr = mmap(NULL, fh->file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fh->fd, 0);

	if (map_addr == (void*)(-1))	
		fh->is_mapped = 0;
	else
	{	
		fh->is_mapped = 1;
		chunk_init(fh->free_chunks, 0, fh->file_size, map_addr);	
		chunk_t* chunk = fh->free_chunks;
		fh->free_chunks->ref_count += 1;
		push_chunk(0, &(fh->free_chunks), &fh);
	}

	return 0;
}

int chunk_pool_unmap(file_handle_t* fh)
{
	chunk_t* chunk = fh->pool[0];
	while (chunk)
	{
		munmap(chunk->addr, chunk->size);
		chunk = chunk->next_iter;
	}

	return 0;
}

int chunk_pool_deinit(file_handle_t* fh)
{
	int i = 0;
	for (i = 0; i <= fh->cur_subpool_num; i++)
		free(fh->pool[i]);
	
	free(fh->pool);

	return 0;
}

