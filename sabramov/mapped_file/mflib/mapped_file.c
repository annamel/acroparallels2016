#include "mapped_file.h"
#include "chunk_manage.h"

#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


mf_handle_t mf_open(const char* pathname)
{	
	if (pathname == NULL)
		return MF_OPEN_FAILED;

	file_handle_t* mf = (file_handle_t*)malloc(sizeof(file_handle_t));
	int error =  file_handle_init(pathname, mf);

	return ((!error) ? mf : MF_OPEN_FAILED);
}

void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle)
{
	if (mf == MF_OPEN_FAILED || offset < 0 || size <= 0 || mapmem_handle == NULL)
	{	
		*mapmem_handle = NULL;
		return MF_MAP_FAILED;
	}

	file_handle_t* fh = mf;

	if (offset > fh->file_size)
	{	
		*mapmem_handle = NULL;
		return MF_MAP_FAILED;
	}

	chunk_t** map_chunk = (chunk_t**)mapmem_handle;	
	chunk_t* chunk = NULL;
	void* addr = NULL;

	if (!chunk_find(mf, &chunk, &addr, offset, size))	
		*mapmem_handle  =  chunk;
	else
	{			
		*mapmem_handle = NULL;
		return MF_MAP_FAILED;
	}

	return addr;
}

ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset)
{
	if (count <= 0 || offset < 0 || buf == NULL || mf == NULL)
	{	
		errno = EINVAL;
		return 0;
	}

	file_handle_t* fh = mf;

	if (fh->file_size < 0)	
		return 0;

	if (offset > fh->file_size)
	{	
		errno = EINVAL;
		return 0;
	}	

	void* addr = NULL;
	chunk_t* chunk;
	chunk_find(mf, &chunk, &addr, offset, count);
	chunk->ref_count -= 1;	

	if ((offset + count) > fh->file_size)
	{
		memcpy(buf, addr, fh->file_size - offset);
		return fh->file_size - offset;
	}
	else
		memcpy(buf, addr, count);

	return count;
}

ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset)
{
	if (count <= 0 || offset < 0 || buf == NULL || mf == NULL)
		return 0;

	file_handle_t* fh = mf;

	if (fh->file_size < 0)
		return 0;

	if (offset > fh->file_size || (offset + count) > fh->file_size)
	{
		errno = EINVAL;
		return 0;
	}	

	void* addr = NULL;
	chunk_t* chunk;
	chunk_find(mf, &chunk, &addr, offset, count);
	chunk->ref_count -=1;
	memcpy(addr, buf, count);

	return count;			
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle)
{
	if (mf == MF_OPEN_FAILED || mapmem_handle == NULL)
		return -1;
	
	file_handle_t* fh = mf;
	chunk_t* unmapped_chunk = mapmem_handle;
	
	if (unmapped_chunk->ref_count == 0)
		return 0;
	
	unmapped_chunk->ref_count -= 1;	

	if (unmapped_chunk->ref_count == 0)
	{
		if (!fast_lookup(unmapped_chunk->pg_round_down_off, unmapped_chunk, fh))
		{
			unmapped_chunk->next = NULL;
			unmapped_chunk->prev = fh->free_chunks_tail;
			
			int res = munmap(unmapped_chunk->addr, unmapped_chunk->size);
			fh->cur_mem_usage -= unmapped_chunk->size;

			if (fh->free_chunks_tail)
				fh->free_chunks_tail->next = unmapped_chunk;
		
			fh->free_chunks_tail = unmapped_chunk;
			
			if (fh->free_chunks == NULL)
				fh->free_chunks = unmapped_chunk;			
		}
	}
	return 0;
}

int mf_close(mf_handle_t mf)
{
	if (mf == MF_OPEN_FAILED)
		return -1;

	file_handle_t* fh = mf;
	chunk_t* ch = fh->free_chunks;

	deinit_hash_table(fh->hash_table);	
	chunk_pool_unmap(fh);
	chunk_pool_deinit(fh);	
	close(fh->fd);
	free(mf);
	
	return 0;	
}

off_t mf_file_size(mf_handle_t mf)
{
	file_handle_t* fh = mf;
	
	if (mf == MF_OPEN_FAILED)
	{
		errno = EINVAL;
		return -1;
	}

	if (fh->fd == -1)
	{
		errno = EBADF;
		return -1;
	}

	return lseek(fh->fd, 0, SEEK_END);
}


