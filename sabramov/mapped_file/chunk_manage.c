#include "chunk_manage.h"

chunk_t* chunk_init(mf_handle_t mf, void* addr, size_t size, off_t offset, int pg_multiple_offset)
{
	chunk_pool_t* pool = mf;
	
	chunk_t* chunk = malloc(sizeof(chunk_t));
	
	if (!chunk)
		return chunk;

	chunk->chunk_size = offset - pg_multiple_offset + size;
	chunk->chunk_addr = addr; 
	chunk->pool = pool;
	chunk->ref_count = 1; 	
	chunk->next = NULL;
	chunk->prev = NULL;
	chunk->is_in_pool = 0;
	chunk->pg_multiple_offset = pg_multiple_offset;

	return chunk;
}

int page_multiple_off(off_t offset)
{
	long int page_size = sysconf(_SC_PAGESIZE);
	int pg_numb = offset / page_size;
	int pg_multiple_offset = pg_numb * page_size;	
	
	return pg_multiple_offset;
} 

mf_mapmem_t* init_mem_handler(void* ptr, chunk_t* chunk)
{
	mf_mapmem_t* mem_id = malloc(sizeof(mf_mapmem_t));
	
	if (!mem_id)	
	{	
		errno = ENOMEM;
		return NULL;
	}

	mem_id->ptr = ptr;
	mem_id->handle = chunk;									
	
	return mem_id; 	
}

mf_mapmem_t *mf_map_system(mf_handle_t mf, off_t offset, size_t size)
{
	chunk_pool_t* pool = mf;
	chunk_t* chunk;
	void* mapped_ptr;
	
	int pg_multiple_offset = page_multiple_off(offset);
	int chunk_size = offset - pg_multiple_offset + size;

	if (!chunk_find(mf, pg_multiple_offset, &chunk))
	{

		void* chunk_addr;

		if (!chunk_acquire(offset, pg_multiple_offset, size, &chunk, &chunk_addr))
			return init_mem_handler(chunk_addr, chunk); 
	}
	
	void* addr = mmap(NULL, chunk_size, PROT_READ | PROT_WRITE, MAP_SHARED, pool->fd, pg_multiple_offset);
	mapped_ptr = (uint8_t*)addr + (offset - pg_multiple_offset);	    // FIXME		
	chunk = chunk_init(mf, addr, size, offset, pg_multiple_offset);
	
	if (!chunk)
		return NULL;

	chunk_add(mf, chunk, pg_multiple_offset);	
		
	return init_mem_handler(mapped_ptr, chunk);			
}

int chunk_add(mf_handle_t mf, chunk_t* chunk, int pg_multiple_offset)
{
	chunk_pool_t* pool = mf;
	insert(pg_multiple_offset, chunk, pool->hash_table);

	return 0;
}

int chunk_add_to_pool(mf_handle_t mf, chunk_t* chunk) //, int pg_multiple_offset)
{
	chunk_pool_t* pool = chunk->pool;
	
	pool->current_mem_usage+=chunk->chunk_size;

	if (pool->max_mem_usage <= pool->current_mem_usage)
	{	
		chunk_pool_free(mf);
		pool->current_mem_usage = chunk->chunk_size;
	}

	chunk_pool_t* pool_ptr = chunk->pool;

	chunk->is_in_pool = 1;

	chunk_add(mf, chunk, chunk->pg_multiple_offset);
	
	if (!pool->mru_chunk)
	{	
		pool->mru_chunk = chunk;			
	}
	else
	{		
		chunk_t* ptr = pool->mru_chunk; 
		pool->mru_chunk = chunk;
		pool->mru_chunk->next = ptr;
		pool->mru_chunk->prev = NULL;
		pool->mru_chunk->next->prev = chunk;			
	}

	return 0;
}


int chunk_pool_free(mf_handle_t mf)
{
	chunk_pool_t* pool = mf;
	chunk_t* chunk = pool->mru_chunk;
		
	
	while(chunk)
	{
		chunk_unmap(chunk);		
		chunk = chunk->next;
		int key = pool->mru_chunk->pg_multiple_offset;
		free(pool->mru_chunk);
		delete(pool->mru_chunk->pg_multiple_offset, pool->hash_table);
		pool->mru_chunk = chunk;
	}
	
	return 0;
}

int chunk_unmap(chunk_t* chunk)
{
	int val = munmap(chunk->chunk_addr, chunk->chunk_size);
	return val;
}

int chunk_pool_init(const char* pathname, size_t max_mem_usage, mf_handle_t mf)
{
	chunk_pool_t* pool = mf;	
	pool->fd = open(pathname, O_RDWR | O_CREAT, S_IRWXU);
	
	if (pool->fd == -1)
		return -1; 
	
	pool->hash_table = init_hash_table();
	
	if (pool->hash_table == NULL)
		return -1;

	pool->mru_chunk = NULL;
	pool->max_mem_usage = max_mem_usage;
	pool->current_mem_usage = 0;
	
	return 0;
}

int chunk_pool_deinit(mf_handle_t mf)
{
	chunk_pool_t* pool = mf;
	deinit_hash_table(pool->hash_table);	
	close(pool->fd);

	return 0;
}

int chunk_find(mf_handle_t mf, off_t multiplied_offset, chunk_t** chunk)
{
	chunk_pool_t* pool = mf; 
	hash_node_t* is_hit;

	if (is_hit = table_lookup(multiplied_offset, pool->hash_table))
	{
		*chunk = is_hit->value;
		return 0;
	}
	else
		return -1;
}

int chunk_acquire(off_t offset, off_t multiplied_offset, size_t size, chunk_t** chunk, void** chunk_addr)
{
	
	if ((*chunk)->chunk_size >= ((offset - multiplied_offset) + size))
	{	
		*chunk_addr = (*chunk)->chunk_addr + (offset - multiplied_offset);
		(*chunk)->ref_count += 1;
		return 0;
	}
	else
	{
		return -1;
	}
}
	
int chunk_release(chunk_t* chunk)  // FIXME: dealing with reference counting			
{
	chunk->ref_count-=1; 
	
	if (!chunk->ref_count)
		return munmap(chunk->chunk_addr, chunk->chunk_size);
	else
		return 0;
}
