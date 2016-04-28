#include "mapped_file.h"
#include "chunk_manage.h"
 
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>


mf_handle_t mf_open(const char* pathname)
{	
	if (pathname == NULL)
		return MF_OPEN_FAILED;

	int max_memory_usage = MAX_MEM_USAGE;

	file_handle_t* mf = (file_handle_t*)malloc(sizeof(file_handle_t));
	int error =  file_handle_init(pathname, max_memory_usage, mf);
	
	if (error)
		return MF_OPEN_FAILED;

	return mf;
}

void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle)
{
	if (mf == MF_OPEN_FAILED || offset < 0 || size <= 0 || mapmem_handle == NULL)		// FIXME: last parameter is necessary ?
		return MF_MAP_FAILED;


	file_handle_t* fh = mf;
	chunk_t** map_chunk = (chunk_t**)mapmem_handle;	

	chunk_t* chunk = NULL;
	void* addr = NULL;

	chunk_find(mf, &chunk, &addr, offset, size);

	*mapmem_handle  =  chunk;

	return addr;
}

ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset)
{
	if (count <= 0 || offset < 0 || buf == NULL)
	{	
		errno = EINVAL;
		return 0;
	}

	int file_size = mf_file_size(mf);

	if (file_size < 0)	
		return 0;

	if (offset > file_size)
	{	
		errno = EINVAL;
		return 0;
	}	
	
	file_handle_t* fh = mf;

	void* addr = NULL;
	chunk_t* chunk;

	chunk_find(mf, &chunk, &addr, offset, count);
	
	chunk->ref_count -= 1;	
	
	if ((offset + count) > file_size)
	{
		memcpy(buf, addr, file_size - offset);
		return file_size - offset;
	}
	else
		memcpy(buf, addr, count);

	return count;
}

ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset)
{
	if (count <= 0 || offset < 0 || buf == NULL)
		return 0;

	int file_size = mf_file_size(mf);

	if (file_size < 0)
		return 0;

	if (offset > file_size)
	{
		errno = EINVAL;
		return 0;
	}	

	file_handle_t* fh = mf;

	void* addr = NULL;
	chunk_t* chunk;

	chunk_find(mf, &chunk, &addr, offset, count);

	chunk->ref_count -=1;
	
	int buf_len = strlen(buf);

	if (buf_len < count)
	{
		memcpy(addr, buf, buf_len);
		return buf_len;
	}
	
	memcpy(addr, buf, count);

	return count;			
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle)
{
	if (mf == MF_OPEN_FAILED)
		return -1;

	file_handle_t* fh = mf;

	chunk_t* unmapped_chunk = mapmem_handle;

	if (unmapped_chunk->ref_count == 0)
		return -1;

	unmapped_chunk->ref_count -= 1;	

	
	if ((unmapped_chunk->ref_count == 0) && (!fast_lookup(unmapped_chunk->pg_multiple_offset, unmapped_chunk, fh)))		
	{
		unmapped_chunk->next = NULL;
		unmapped_chunk->prev = fh->free_chunks_tail;

		if (fh->free_chunks_tail)
			fh->free_chunks_tail->next = unmapped_chunk;
	
		fh->free_chunks_tail = unmapped_chunk;
		
		if (fh->free_chunks == NULL)
			fh->free_chunks = unmapped_chunk;
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


