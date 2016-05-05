#include "chunk_manage.h"
#include "mapped_file.h"

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int page_multiple_off(off_t offset)
{
	long int page_size = sysconf(_SC_PAGESIZE);
	int pg_numb = offset / page_size;
	int pg_multiple_offset = pg_numb * page_size;	
	
	return pg_multiple_offset;
}

int page_multiple_off_up(off_t offset, size_t size)
{
	long int page_size = sysconf(_SC_PAGESIZE);
	int pg_numb_down = (offset + size) / page_size;
	int pg_multiple_offset_up = (pg_numb_down + 1) * page_size;

	return pg_multiple_offset_up; 
} 

int chunk_init(chunk_t* chunk, off_t pg_multiple_offset, size_t size, void* addr)
{
	chunk->pg_multiple_offset = pg_multiple_offset;
	chunk->size = size;
	chunk->addr = addr;
	chunk->ref_count = 0;

	return 0;
}

int is_chunk_reusable(chunk_t* chunk, int  chunk_size)
{
	if (chunk->size >= chunk_size)
		return 1;
		
	return 0;
}

void chunk_reuse(chunk_t* chunk, off_t offset, int pg_multiple_offset, void** chunk_addr)
{
		*chunk_addr = chunk->addr + (offset - pg_multiple_offset);			
}

int chunk_find(mf_handle_t mf, chunk_t** chunk, void** addr, off_t offset, size_t size)
{
	file_handle_t* fh = mf;

	int file_size = mf_file_size(mf);

	int pg_multiple_offset = page_multiple_off(offset);
	int pg_multiple_offset_up = page_multiple_off_up(offset, size);			
	int chunk_size = 0;
	
	if (file_size)
	{
		if (pg_multiple_offset_up > file_size)
			chunk_size = file_size - pg_multiple_offset;			 
		else
		{	
			chunk_size = pg_multiple_offset_up - pg_multiple_offset;
		}
	}
	else
	{
		chunk_size = pg_multiple_offset_up - pg_multiple_offset;	
	}


	*chunk = (chunk_t*)(intptr_t)table_lookup(pg_multiple_offset, fh);

	if (*chunk)
	{				
		if (is_chunk_reusable(*chunk, chunk_size))
		{

			chunk_reuse(*chunk, offset, pg_multiple_offset, addr);
 			
			(*chunk)->ref_count += 1;
			
			return 0;
		}
		else
		{

			void* remap_addr = (void*)(intptr_t)mremap((*chunk)->addr, (*chunk)->size, chunk_size, 0);	// FIXME: MREMAP_MAYMOVE
																										// need to fix pointer casting	
			if (remap_addr != (void*)(-1))
			{
				(*chunk)->addr = remap_addr;
				(*chunk)->size = chunk_size;
 
				chunk_reuse((*chunk), offset, pg_multiple_offset, addr);

				(*chunk)->ref_count += 1;				
				
				return 0;
			}	
		
		}
	}		

	void* map_addr = mmap(NULL, chunk_size, PROT_READ | PROT_WRITE, MAP_SHARED, fh->fd, pg_multiple_offset);
	*addr = (uint8_t*)map_addr + (offset - pg_multiple_offset);	    		

	if (!fh->free_chunks)
		chunk_subpool_init(fh);

	chunk_init(fh->free_chunks, pg_multiple_offset, chunk_size, map_addr);	
	
	*chunk = fh->free_chunks;

	fh->free_chunks->ref_count += 1;

	push_chunk(pg_multiple_offset, &(fh->free_chunks), &fh);		

	return 0;			
}

int chunk_subpool_init(file_handle_t* fh)
{
	if ((fh->cur_subpool_numb + 1) == fh->subpools_numb)
	{
		chunk_t** new_pool = (chunk_t**)realloc(fh->pool, (fh->subpools_numb * 2) * sizeof(chunk_t*));

		if (!new_pool)
			return -1;

		fh->pool = new_pool;
		fh->subpools_numb *= 2;
	}
	
	fh->cur_subpool_numb += 1;
		
	fh->pool[fh->cur_subpool_numb] = (chunk_t*)malloc(sizeof(chunk_t) * INIT_CHUNKS_NUMB); 

	if (!fh->pool[fh->cur_subpool_numb])
		return -1;
	
	chunk_t* chp = fh->pool[fh->cur_subpool_numb];

	chunk_t* prev_chp = fh->pool[fh->cur_subpool_numb - 1];

	chp[0].prev = &(prev_chp[INIT_CHUNKS_NUMB - 1]);
	chp[0].next = &(chp[1]);
	chp[0].prev_iter = &(prev_chp[INIT_CHUNKS_NUMB - 1]);
	chp[0].next_iter = &(chp[1]);

	prev_chp[INIT_CHUNKS_NUMB - 1].next_iter = &(chp[0]);

	int i = 0;

	for (i = 1; i < INIT_CHUNKS_NUMB - 1; i++)
	{
		chp[i].next = &(chp[i+1]);	
		chp[i].prev = &(chp[i-1]);	
		chp[i].next_iter = &(chp[i+1]);	
		chp[i].prev_iter = &(chp[i-1]);
	}

	chp[INIT_CHUNKS_NUMB - 1].next = NULL;	
	chp[INIT_CHUNKS_NUMB - 1].prev = &(chp[INIT_CHUNKS_NUMB - 2]);
	chp[INIT_CHUNKS_NUMB - 1].next_iter = NULL;	
	chp[INIT_CHUNKS_NUMB - 1].prev_iter = &(chp[INIT_CHUNKS_NUMB - 2]);	

	fh->free_chunks = &(chp[0]);
	fh->free_chunks_tail = &(chp[INIT_CHUNKS_NUMB - 1]);
	
	return 0;
}

int file_handle_init(const char* pathname, int max_mem_usage, mf_handle_t mf)
{
	file_handle_t* fh = mf;

	fh->fd = open(pathname, O_RDWR | O_CREAT, S_IRWXU);

	if (fh->fd == -1)
		return -1;

	fh->hash_table = (chunk_t**)(intptr_t)init_hash_table(); 

	fh->max_mem_usage = max_mem_usage;
	fh->cur_mem_usage = 0;		// FIXME:
	fh->subpools_numb = INIT_POOL_LENGTH;

	fh->pool = (chunk_t**)malloc(sizeof(chunk_t*) * INIT_POOL_LENGTH);

	if (!fh->pool)
		return -1;

	fh->cur_subpool_numb = 0;

	fh->pool[0] = (chunk_t*)malloc(sizeof(chunk_t) * INIT_CHUNKS_NUMB);

	chunk_t* chp = fh->pool[0];
	
	fh->free_chunks = &(chp[0]);

	chp[0].prev = NULL;
	chp[0].next = &(chp[1]);
	chp[0].prev_iter = NULL;
	chp[0].next_iter = &(chp[1]);

	int i = 0;

	for (i = 1; i < INIT_CHUNKS_NUMB - 1; i++)
	{
		chp[i].next = &(chp[i+1]);	
		chp[i].prev = &(chp[i-1]);	
		chp[i].next_iter = &(chp[i+1]);	
		chp[i].prev_iter = &(chp[i-1]);
	}

	chp[INIT_CHUNKS_NUMB - 1].next = NULL;	
	chp[INIT_CHUNKS_NUMB - 1].prev = &(chp[INIT_CHUNKS_NUMB - 2]);
	chp[INIT_CHUNKS_NUMB - 1].next_iter = NULL;	
	chp[INIT_CHUNKS_NUMB - 1].prev_iter = &(chp[INIT_CHUNKS_NUMB - 2]);	

	fh->free_chunks_tail = &(chp[INIT_CHUNKS_NUMB - 1]);

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
	
	for (i = 0; i <= fh->cur_subpool_numb; i++)
	{
		free(fh->pool[i]);
	}
	
	free(fh->pool);

	return 0;
}

