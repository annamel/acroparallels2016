#include "mapped_file_internal.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

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

#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef ALIGN
	#define ALIGN(val,size) (((val) + (size) - 1)& (~((size) - 1)))
#endif // ALIGN

#ifdef MULTITHREADING
	#define LOCK_READ(name)											\
	do																\
	{																\
		CHECK(!pthread_rwlock_wrlock(&file->rwlock_ ## name));		\
	} while (0)
	#define LOCK_WRITE(name)										\
	do																\
	{																\
		CHECK(!pthread_rwlock_wrlock(&file->rwlock_ ## name));		\
	} while (0)
	#define UNLOCK(name)											\
	do																\
	{																\
		CHECK(!pthread_rwlock_unlock(&file->rwlock_ ## name));		\
	} while (0)
#else
	#define LOCK_READ(name)
	#define LOCK_WRITE(name)
	#define UNLOCK(name)
#endif // MULTITHREADING

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

static uint64_t mapmem_hashfunction(uint64_t key)
{
	return key;
}

static int mapmem_destroy(uint64_t key, void* value, void* data)
{
	mapped_chunk_t* chunk = (mapped_chunk_t*) value;

	chunk->ref_count--;

	mapped_file_t* file = (mapped_file_t*) data;
		
	unmap_internal(chunk->data, chunk->offset, chunk->size, file->page_size, file->file_size);

	invalidate_data(chunk, sizeof (mapped_chunk_t));

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

	hashtable_init(&file->chunks, CHUNKS_HASHTABLE_SIZE, mapmem_hashfunction);

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
	assert(IS_POWER_OF_2(file->page_size));
	if (file->page_size == (size_t) -1)
	{
		invalidate_data(file, sizeof (mapped_file_t));
		free(file);
		close(file->fd);
		RETURN_FAIL(MF_OPEN_FAILED);
	}
	file->chunk_size = (MAP_CHUNK_SIZE / file->page_size) * file->page_size;
	assert(IS_POWER_OF_2(file->chunk_size));
	if (!file->chunk_size)
	{
		invalidate_data(file, sizeof (mapped_file_t));
		free(file);
		close(file->fd);
		RETURN_ERRNO(EINVAL, MF_OPEN_FAILED);
	}

	file->free_chunks = NULL;

	file->data = map_internal(0, file->file_size, file->fd, file->page_size, file->file_size);
	if (file->data == MAP_FAILED)
	{
		errno = 0;
		file->fully_mapped = 0;
		file->data = NULL;

#ifdef MULTITHREADING
		CHECK(!pthread_rwlock_init(&file->rwlock_map, NULL));
		CHECK(!pthread_rwlock_init(&file->rwlock_readwrite, NULL));
#endif // MULTITHREADING
	}
	else
	{
		file->fully_mapped = 1;
		file->offset = 0;
		file->size = file->file_size;
	}

	RETURN((mf_handle_t) file);
}

int mf_close(mf_handle_t mf) // this function must be called after all multithread work
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

#ifdef MULTITHREADING
	CHECK(!pthread_rwlock_destroy(&file->rwlock_map));
	CHECK(!pthread_rwlock_destroy(&file->rwlock_readwrite));
#endif // MULTITHREADING

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

	LOCK_READ(map);

	uint64_t key = TO_KEY(aligned_offset, aligned_size);
	mapped_chunk_t* chunk = hashtable_get(&file->chunks, key);

	if (chunk)
	{
		if (chunk->ref_count == 0)
		{
			if (chunk->prev)
				chunk->prev->next = chunk->next;
			if (chunk->next)
				chunk->next->prev = chunk->prev;
		}
		chunk->ref_count++;
		*mapmem_handle = (mf_mapmem_handle_t) chunk;
		void* data = (void*) (&((char*) chunk->data)[offset_delta]);
		UNLOCK(map);
		RETURN(data);
	}
	
	UNLOCK(map);
	LOCK_WRITE(map);

	void* pointer;
	while ((pointer = map_internal(aligned_offset, aligned_size, file->fd, file->page_size, file->file_size)) == MAP_FAILED)
	{
		if (file->free_chunks == NULL)
			RETURN_FAIL(MF_MAP_FAILED);

		mapped_chunk_t* free_chunk = file->free_chunks;
		file->free_chunks = file->free_chunks->next;

		key = TO_KEY(free_chunk->offset, free_chunk->size);

		if (!hashtable_remove(&file->chunks, key, mapmem_destroy, file))
		{
			invalidate_data(chunk, sizeof (mapped_chunk_t));
			free(chunk);
			UNLOCK(map);
			RETURN_ERRNO(EINVAL, MF_MAP_FAILED);
		}
	}

	chunk = malloc(sizeof (mapped_chunk_t));
	if (!chunk)
	{
		CHECK(unmap_internal(pointer, aligned_offset, aligned_size, file->page_size, file->file_size));
		UNLOCK(map);
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);
	}

	void* data = (void*) (&((char*) pointer)[offset_delta]);
	chunk->data = pointer;
	chunk->size = aligned_size;
	chunk->offset = aligned_offset;
	chunk->ref_count = 1;

	hashtable_add(&file->chunks, key, chunk);

	*mapmem_handle = (mf_mapmem_handle_t) chunk;
	UNLOCK(map);
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

	LOCK_WRITE(map);
	mapped_chunk_t* chunk = (mapped_chunk_t*) mapmem_handle;

	chunk->ref_count--;
	if (chunk->ref_count == 0)
	{
		chunk->next = file->free_chunks;
		chunk->prev = NULL;
		if (file->free_chunks)
			file->free_chunks->prev = chunk;
		file->free_chunks = chunk;
	}

	UNLOCK(map);

	RETURN(0);
}

static int map_for_read_write(mf_handle_t mf, off_t offset, size_t size)
{
	GET_MAPPED_FILE(-1)

	LOCK_READ(readwrite);
	if (file->data && offset >= file->offset && offset + size <= file->offset + file->size)
	{
		RETURN(0);
	}

	LOCK_WRITE(readwrite);

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
	UNLOCK(readwrite);

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
	UNLOCK(readwrite);

	RETURN(count);
}
