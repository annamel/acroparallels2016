#include "mapped_file_internal.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

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

#ifdef DEBUG_MODE
	#define CHECK(val)												\
	do																\
	{																\
		if (!(val))													\
			abort();												\
	} while (0)
#else
	#define CHECK(val)												\
	do																\
	{																\
		val;														\
	} while (0)
#endif // DEBUG_MODE

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

#ifndef ALIGN
	#define ALIGN(val,size) (((val) + (size) - 1)& (~((size) - 1)))
#endif // ALIGN

static void invalidate_data(void* data, size_t size)
{
#ifdef DEBUG_MODE
	int i;
	for (i = 0; i < size / sizeof (int32_t); i++)
		((int32_t*) data)[i] = 0xDEADBABA;
#endif // DEBUG_MODE
}

static void* map_internal(off_t offset, size_t size, int fd, size_t page_size, size_t file_size)
{
	if (size + offset > file_size)
		size = file_size - offset;
#ifndef MMAP_PROTECTION
	void* pointer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
	return pointer;
#else
	size_t first_mmap_size = ALIGN(size, page_size);

	void* pointer = mmap(NULL, first_mmap_size + 2 * page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (pointer == MAP_FAILED)
		return MAP_FAILED;

	void* data_pointer = mmap(&(((char*) pointer)[page_size]), size, PROT_READ | PROT_WRITE, 
		MAP_SHARED | MAP_FIXED, fd, offset);
	if (data_pointer == MAP_FAILED)
	{
		munmap(pointer, first_mmap_size + 2 * page_size);
		return MAP_FAILED;
	}
	if (&(((char*) pointer)[page_size]) != data_pointer)
	{
		munmap(pointer, first_mmap_size + 2 * page_size);
		munmap(data_pointer, size);
		return MAP_FAILED;
	}

	int i;
	for (i = 0; i < page_size / sizeof (int32_t); i++)
	{
		((int32_t*) pointer)[i] = 0xDEADBABA;
		((int32_t*) pointer)[(first_mmap_size + 2 * page_size) / sizeof (int32_t) - i - 1] = 0xDEADBABA;
	}
	return data_pointer;
#endif // MMAP_PROTECTION
}

static int unmap_internal(void* addr, off_t offset, size_t size, size_t page_size, size_t file_size)
{
	if (size + offset > file_size)
		size = file_size - offset;
#ifndef DEBUG_MODE
	return munmap(addr, size);
#else
	size_t first_mmap_size = ALIGN(size, page_size);
	int err_code1 = munmap(addr, first_mmap_size);
	int err_code2 = munmap(&(((char*) addr)[-page_size]), size);
	return (err_code1 == -1 || err_code2 == -1) ? -1 : 0;
#endif // DEBUG_MODE
}

static size_t mapmem_hashfunction(void* key)
{
	mapped_chunk_key_t* mapmem = (mapped_chunk_key_t*) key;
	return mapmem->offset ^ mapmem->size;
}

int mapmem_comparator(void* key1, void* key2)
{
	mapped_chunk_key_t* mapmem1 = (mapped_chunk_key_t*) key1;
	mapped_chunk_key_t* mapmem2 = (mapped_chunk_key_t*) key2;

	return mapmem1->offset == mapmem2->offset && mapmem1->size == mapmem2->size;
}

static int mapmem_destroy(void* key, void* value, void* data)
{
	mapped_chunk_key_t* chunk_key = (mapped_chunk_key_t*) key;
	mapped_chunk_t* chunk = (mapped_chunk_t*) value;

	chunk->ref_count--;
	if (chunk->ref_count != 0) // TODO: assert
		return 0;	

	mapped_file_t* file = (mapped_file_t*) data;

	file->mapped_memory_usage -= chunk->size;
		
	unmap_internal(chunk->data, chunk->offset, chunk->size, file->page_size, file->file_size);

	invalidate_data(chunk_key, sizeof (mapped_chunk_key_t));
	invalidate_data(chunk, sizeof (mapped_chunk_t));

	free(chunk_key);
	free(chunk);

	return 1;
}

static int ispowerof2(unsigned int x)
{
	return x && !(x & (x - 1));
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
	assert(ispowerof2(file->page_size));
	if (file->page_size == (size_t) -1)
	{
		invalidate_data(file, sizeof (mapped_file_t));
		free(file);
		close(file->fd);
		RETURN_FAIL(MF_OPEN_FAILED);
	}
	file->chunk_size = (MAP_CHUNK_SIZE / file->page_size) * file->page_size;
	assert(ispowerof2(file->chunk_size));
	if (!file->chunk_size)
	{
		invalidate_data(file, sizeof (mapped_file_t));
		free(file);
		close(file->fd);
		RETURN_ERRNO(EINVAL, MF_OPEN_FAILED);
	}

	file->mapped_memory_usage = 0;
	file->free_chunks = NULL;

	file->data = map_internal(0, file->file_size, file->fd, file->page_size, file->file_size);
	if (file->data == MAP_FAILED)
	{
		errno = 0;
		file->fully_mapped = 0;
		file->data = NULL;
	}
	else
	{
		file->fully_mapped = 1;
		file->offset = 0;
		file->size = file->file_size;
	}

	RETURN((mf_handle_t) file);
}

int mf_close(mf_handle_t mf)
{
	GET_MAPPED_FILE(-1)

	if (close(file->fd) == -1)
		RETURN_FAIL(-1);
	
	hashtable_destroy(&file->chunks, mapmem_destroy, file);

	if (file->data)
	{
		if (unmap_internal(file->data, file->offset, file->size, file->page_size, file->file_size) == -1)
			RETURN_FAIL(-1);
	}

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

	if (file->fully_mapped)
	{
		*mapmem_handle = (mf_mapmem_handle_t) 0xDEADBABA;
		RETURN(&(((char*) file->data)[offset]));
	}

	size_t aligned_offset = ALIGN(size, file->chunk_size);
	size_t offset_delta = offset - aligned_offset;
	size += offset_delta;
	size_t aligned_size = ALIGN(size, file->chunk_size);
	if (size % file->chunk_size != 0)
		aligned_size += file->chunk_size;

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

	file->mapped_memory_usage += size;

	mapped_chunk_key_t* key_ptr = malloc(sizeof (mapped_chunk_key_t));
	if (!key_ptr)
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);
	key_ptr->size = aligned_size;
	key_ptr->offset = aligned_offset;
	
	void* pointer;
	while ((pointer = map_internal(aligned_offset, aligned_size, file->fd, file->page_size, file->file_size)) == MAP_FAILED)
	{
		if (file->free_chunks == NULL)
			RETURN_FAIL(MF_MAP_FAILED);

		mapped_chunk_t* free_chunk = file->free_chunks;
		file->free_chunks = file->free_chunks->next;

		key.size = free_chunk->size;
		key.offset = free_chunk->offset;

		if (!hashtable_remove(&file->chunks, &key, mapmem_destroy, file))
		{
			invalidate_data(chunk, sizeof (mapped_chunk_t));
			free(chunk);
			RETURN_ERRNO(EINVAL, MF_MAP_FAILED);
		}
	}

	chunk = malloc(sizeof (mapped_chunk_t));
	if (!chunk)
	{
		CHECK(unmap_internal(pointer, aligned_offset, aligned_size, file->page_size, file->file_size));
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);
	}

	void* data = (void*) (&((char*) pointer)[offset_delta]);
	chunk->data = pointer;
	chunk->size = aligned_size;
	chunk->offset = aligned_offset;
	chunk->ref_count = 1;

	hashtable_add(&file->chunks, key_ptr, chunk);

	*mapmem_handle = (mf_mapmem_handle_t) chunk;
	RETURN(data);
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle)
{
	GET_MAPPED_FILE(-1)

	if (!mapmem_handle)
		RETURN_ERRNO(EINVAL, -1);
	if (file->fully_mapped)
	{
		if (mapmem_handle == (mf_mapmem_handle_t) 0xDEADBABA)
			RETURN(0);
		else
			RETURN_ERRNO(EINVAL, -1);
	}

	mapped_chunk_t* chunk = (mapped_chunk_t*) mapmem_handle;

	chunk->ref_count--;
	if (chunk->ref_count == 0)
	{
		chunk->next = file->free_chunks;
		file->free_chunks = chunk;
	}

	RETURN(0);
}

static int map_for_read_write(mf_handle_t mf, off_t offset, size_t size)
{
	GET_MAPPED_FILE(-1)

	if (file->data && offset >= file->offset && offset + size <= file->offset + file->size)
		RETURN(0);

	if (file->data)
		CHECK(unmap_internal(file->data, file->offset, file->size, file->page_size, file->file_size));

	size = max(READ_WRITE_MIN_SIZE, size);

	size_t aligned_offset = ALIGN(size, file->chunk_size);
	size_t offset_delta = offset - aligned_offset;
	size += offset_delta;

	void* pointer = map_internal(aligned_offset, size, file->fd, file->page_size, file->file_size);
	
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

	if (map_for_read_write(mf, offset, count) == -1)
		RETURN_FAIL(-1);

	memcpy(buf, &(((char*) file->data)[offset - file->offset]), count);

	if (count >= UNMAP_READ_WRITE_SIZE && !file->fully_mapped)
	{
		CHECK(unmap_internal(file->data, file->offset, file->size, file->page_size, file->file_size));
		file->data = NULL;
	}

	RETURN(count);
}

ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset)
{
	GET_MAPPED_FILE(-1)

	if (count == 0 || offset >= file->file_size)
		RETURN_ERRNO(EINVAL, -1);
	if (offset + count > file->file_size)
		count = file->file_size - offset;

	if (map_for_read_write(mf, offset, count) == -1)
		RETURN_FAIL(-1);

	memcpy(&(((char*) file->data)[offset - file->offset]), buf, count);

	if (count >= UNMAP_READ_WRITE_SIZE && !file->fully_mapped)
	{
		CHECK(unmap_internal(file->data, file->offset, file->size, file->page_size, file->file_size));
		file->data = NULL;
	}

	RETURN(count);
}
