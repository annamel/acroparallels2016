#include "mapped_file_internal.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifdef WITH_LOGGER
	#define RETURN(val)												\
	do																\
	{																\
		logi("Function %s end", __FUNCTION__);						\
		return val;													\
	} while (0)

	#define RETURN_FAIL(val)										\
	do																\
	{																\
		loge("Function %s end with errno %s", 						\
			__FUNCTION__, strerror(errno));							\
		return val;													\
	} while (0)

	#define RETURN_ERRNO(error, val)								\
	do																\
	{																\
		errno = error;												\
		loge("Function %s end with errno %s", 						\
			__FUNCTION__, strerror(errno));							\
		return val;													\
	} while (0)

	#define BEGIN_FUNCTION()										\
		logi("Function %s begin", __FUNCTION__);
#else
	#define RETURN(val)												\
	do																\
	{																\
		return val;													\
	} while (0)

	#define RETURN_FAIL(val)										\
	do																\
	{																\
		return val;													\
	} while (0)

	#define RETURN_ERRNO(error, val)								\
	do																\
	{																\
		errno = error;												\
		return val;													\
	} while (0)

	#define BEGIN_FUNCTION()
#endif // WITH_LOGGER

#define GET_MAPPED_FILE(retval) 									\
	BEGIN_FUNCTION()												\
	mapped_file_t* file = (mapped_file_t*) mf;						\
	if (!file)														\
		RETURN_ERRNO(EINVAL, retval);


#ifdef WITH_LOGGER
	__attribute__((constructor))
	static void init()
	{
		log_init(LOG_INFO, true);
	}

	__attribute__((destructor))
	static void destroy() 
	{
		log_destroy();
	}
#endif

static void invalidate_data(void* data, size_t size)
{
#ifdef DEBUG_MODE
	int i;
	for (i = 0; i < size / sizeof (int); i++)
		((int*) data)[i] = 0xDEADBABA;
#endif // DEBUG_MODE
}

static size_t mapmem_hashfunction(void* key)
{
	mapped_chunk_key_t* mapmem = (mapped_chunk_key_t*) key;
	return mapmem->offset;
}

int mapmem_comparator(void* key1, void* key2)
{
	mapped_chunk_key_t* mapmem1 = (mapped_chunk_key_t*) key1;
	mapped_chunk_key_t* mapmem2 = (mapped_chunk_key_t*) key2;

	return mapmem1->offset == mapmem2->offset && mapmem1->size == mapmem2->size;
}

static int mapmem_destroy(void* key, void* value)
{
	mapped_chunk_key_t* chunk_key = (mapped_chunk_key_t*) key;
	mapped_chunk_t* chunk = (mapped_chunk_t*) value;

	chunk->ref_count--;
	if (chunk->ref_count != 0)
		return 0;

	munmap(chunk->data, chunk_key->size);

	invalidate_data(chunk_key, sizeof (mapped_chunk_key_t));
	invalidate_data(chunk, sizeof (mapped_chunk_t));

	free(chunk_key);
	free(chunk);

	return 1;
}

mf_handle_t mf_open(const char* pathname)
{
	BEGIN_FUNCTION()

	if (!pathname)
		RETURN_ERRNO(EINVAL, MF_OPEN_FAILED);
	
	mapped_file_t* file = malloc(sizeof (mapped_file_t));
	if (!file)
		RETURN_ERRNO(ENOMEM, MF_OPEN_FAILED);

	file->fd = open(pathname, O_RDWR);
	if (file->fd == -1)
	{
		invalidate_data(file, sizeof (mapped_file_t));
		free(file);
		RETURN_FAIL(MF_OPEN_FAILED);
	}

	hashtable_init(&file->chunks, CHUNKS_HASHTABLE_SIZE, mapmem_hashfunction, mapmem_comparator);

	struct stat st;
	if (fstat(file->fd, &st) == -1)
	{
		invalidate_data(file, sizeof (mapped_file_t));
		free(file);
		close(file->fd);
		RETURN_FAIL(MF_OPEN_FAILED);
	}
	file->file_size = st.st_size;
	file->page_size = (size_t) sysconf(_SC_PAGE_SIZE);
	if (file->page_size == (size_t) -1)
	{
		invalidate_data(file, sizeof (mapped_file_t));
		free(file);
		close(file->fd);
		RETURN_FAIL(MF_OPEN_FAILED);
	}

	file->data = NULL;

	RETURN((mf_handle_t) file);
}

int mf_close(mf_handle_t mf)
{
	GET_MAPPED_FILE(-1)

	if (close(file->fd) == -1)
		RETURN_FAIL(-1);
	
	hashtable_destroy(&file->chunks, mapmem_destroy);

	if (file->data)
		munmap(file->data, file->size);

	invalidate_data(file, sizeof (mapped_file_t));
	free(file);

	RETURN(0);
}

off_t mf_file_size(mf_handle_t mf)
{
	GET_MAPPED_FILE(-1)
	RETURN(file->file_size);
}

void* mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t* mapmem_handle)
{
	GET_MAPPED_FILE(MF_MAP_FAILED)

	if (!mapmem_handle)
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);
	if (offset + size > file->file_size || size == 0)
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);

	size_t aligned_offset = (offset / file->page_size) * file->page_size;
	size_t offset_delta = offset - aligned_offset;
	size += offset_delta;
	size_t aligned_size = (size / file->page_size) * file->page_size;

	mapped_chunk_key_t key = 
	{
		.size = aligned_size, 
		.offset = aligned_offset
	};
	mapped_chunk_t* chunk = hashtable_get(&file->chunks, &key);

	if (chunk)
	{
		chunk->ref_count++;
		*mapmem_handle = (mf_mapmem_handle_t) chunk;
		void* data = (void*) (&((char*) chunk->data)[offset_delta]);
		RETURN(data);
	}

	mapped_chunk_key_t* key_ptr = malloc(sizeof (mapped_chunk_key_t));
	if (!key_ptr)
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);
	key_ptr->size = aligned_size;
	key_ptr->offset = aligned_offset;

	void* pointer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, file->fd, aligned_offset);
	if (pointer == MAP_FAILED)
		RETURN_FAIL(MF_MAP_FAILED);

	chunk = malloc(sizeof (mapped_chunk_t));
	if (!chunk)
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);

	void* data = (void*) (&((char*) pointer)[offset_delta]);
	chunk->data = pointer;
	chunk->size = aligned_size;
	chunk->offset = aligned_offset;
	chunk->ref_count = 1;

	hashtable_add(&file->chunks, key_ptr, chunk);

	*mapmem_handle = (mf_mapmem_handle_t) chunk;
	RETURN(chunk->data);
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle)
{
	GET_MAPPED_FILE(-1)

	if (!mapmem_handle)
		RETURN_ERRNO(EINVAL, -1);

	mapped_chunk_t* chunk = (mapped_chunk_t*) mapmem_handle;
	mapped_chunk_key_t key = 
	{
		.size = chunk->size, 
		.offset = chunk->offset
	};

	if (!hashtable_remove(&file->chunks, &key, mapmem_destroy))
	{
		invalidate_data(chunk, sizeof (mapped_chunk_t));
		free(chunk);
		RETURN_ERRNO(EINVAL, -1);
	}

	RETURN(0);
}

static int mf_map_internal(mf_handle_t mf, off_t offset, size_t size)
{
	GET_MAPPED_FILE(-1)

	if (file->data && offset >= file->offset && offset + size <= file->offset + file->size)
		RETURN(0);

	size = max(READ_WRITE_MIN_SIZE, size);

	size_t aligned_offset = (offset / file->page_size) * file->page_size;
	size_t offset_delta = offset - aligned_offset;
	size += offset_delta;

	void* pointer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, file->fd, aligned_offset);
	if (pointer == MAP_FAILED)
		RETURN_FAIL(-1);

	file->data = pointer;
	file->size = size;
	file->offset = aligned_offset;

	RETURN(0);
}

ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset)
{
	GET_MAPPED_FILE(-1)

	if (count == 0 || offset >= file->file_size)
		RETURN_ERRNO(EINVAL, -1);
	if (offset + count > file->file_size)
		count = file->file_size - offset;

	if (mf_map_internal(mf, offset, count) == -1)
		RETURN_FAIL(-1);

	memcpy(buf, &(((char*) file->data)[offset - file->offset]), count);

	if (count >= UNMAP_READ_WRITE_SIZE)
		munmap(file->data, file->size);

	RETURN(count);
}

ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset)
{
	GET_MAPPED_FILE(-1)

	if (count == 0 || offset >= file->file_size)
		RETURN_ERRNO(EINVAL, -1);
	if (offset + count > file->file_size)
		count = file->file_size - offset;

	if (mf_map_internal(mf, offset, count) == -1)
		RETURN_FAIL(-1);

	memcpy(&(((char*) file->data)[offset - file->offset]), buf, count);

	if (count >= UNMAP_READ_WRITE_SIZE)
	{
		munmap(file->data, file->size);
		file->data = NULL;
	}

	RETURN(count);
}
