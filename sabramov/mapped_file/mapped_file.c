#include "mapped_file.h"
#include "chunk_manage.h"

mf_handle_t mf_open(const char* pathname, size_t max_memory_usage)
{
	if (pathname == NULL || max_memory_usage >= MAX_MEM_USAGE)
		return MF_OPEN_FAILED;

	if (max_memory_usage == 0)
		max_memory_usage = MAX_MEM_USAGE;

	chunk_pool_t* mf = (chunk_pool_t*)malloc(sizeof(chunk_pool_t));
	int error = chunk_pool_init(pathname, max_memory_usage, mf);

	if (error)
		return NULL;

	return mf;
}

mf_mapmem_t *mf_map(mf_handle_t mf, off_t offset, size_t size)
{
	chunk_pool_t* pool = mf;
	chunk_t* chunk;
	void* mapped_ptr;
	
	int pg_multiple_offset = page_multiple_off(offset);
	int chunk_size = offset - pg_multiple_offset + size;

	if ((!chunk_find(mf, pg_multiple_offset, &chunk)) && (!chunk->is_in_pool))
	{
		void* chunk_addr;

		if (!chunk_acquire(offset, pg_multiple_offset, size, &chunk, &chunk_addr))
			return init_mem_handler(chunk_addr, chunk); 
	}
	
	void* addr = mmap(NULL, chunk_size, PROT_READ | PROT_WRITE, MAP_SHARED, pool->fd, pg_multiple_offset);
	mapped_ptr = (uint8_t*)addr + (offset - pg_multiple_offset);	    		
	chunk = chunk_init(mf, addr, size, offset, pg_multiple_offset);
	chunk_add(mf, chunk, pg_multiple_offset);	
		
	return init_mem_handler(mapped_ptr, chunk);			
}


ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void* buf)
{
	chunk_pool_t* pool = mf;
	mf_mapmem_t* alloc_mem = mf_map_system(mf, offset, size);
	
	if (!alloc_mem)
	{
		errno = ENOMEM;
		return -1;
	}
	
	chunk_t* chunk = alloc_mem->handle;	
	chunk->ref_count -=1;	

	if (chunk->ref_count == 0 && chunk->is_in_pool == 0)
		chunk_add_to_pool(mf, chunk);
	
	memcpy(buf, alloc_mem->ptr, size);
	
	free(alloc_mem);

	return size;
}


int mf_unmap(mf_mapmem_t *mm)
{

	chunk_t* unmapped_chunk = mm->handle;
	
	if (unmapped_chunk->ref_count == 0)
		return -1;

	unmapped_chunk->ref_count -= 1;
	
	if (unmapped_chunk->ref_count == 0 && unmapped_chunk->is_in_pool == 0)
		chunk_add_to_pool(unmapped_chunk->pool, unmapped_chunk);	
	
	free(mm);
	
	return 0;
}

int mf_close(mf_handle_t mf)
{
	if (!mf)
		return -1;

	chunk_pool_free(mf);
	chunk_pool_t* pool = mf;
	chunk_pool_deinit(mf);
	free(mf);
	
	return 0;	
}

ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void* buf)
{
	chunk_pool_t* pool = mf;
	mf_mapmem_t* alloc_mem = mf_map(mf, offset, size);
	
	if (!alloc_mem)
	{
		errno = ENOMEM;
		return -1;
	}
	
	chunk_t* chunk = alloc_mem->handle;	
	chunk->ref_count -=1;

	if(chunk->ref_count == 0 && chunk->is_in_pool == 0)
		chunk_add_to_pool(mf, chunk);
			
	memcpy(alloc_mem->ptr, buf, size);
	
	free(alloc_mem);

	return size;			
}


ssize_t mf_file_size(mf_handle_t mf)
{
	chunk_pool_t* pool = mf;
	
	if (mf == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	return lseek(pool->fd, 0, SEEK_END);
}


