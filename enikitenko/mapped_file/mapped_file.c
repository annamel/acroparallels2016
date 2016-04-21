#include "mapped_file_internal.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

	#define BEGIN_FUNCTION											\
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

	#define BEGIN_FUNCTION
#endif // WITH_LOGGER

#define GET_MAPPED_FILE(retval) 									\
	BEGIN_FUNCTION													\
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

static size_t mapmem_hashfunction(void* key)
{
	mapped_key_t* mapmem = (mapped_key_t*) key;
	return mapmem->offset;
}

int mapmem_comparator(void* key1, void* key2)
{
	mapped_key_t* mapmem1 = (mapped_key_t*) key1;
	mapped_key_t* mapmem2 = (mapped_key_t*) key2;

	return mapmem1->offset == mapmem2->offset && mapmem1->size == mapmem2->size;
}

static void mapmem_destroy(void* key, void* value)
{
	mapped_key_t* mapped_key = (mapped_key_t*) key;
	mapped_chunk_t* mapped_value = (mapped_chunk_t*) value;

	mapped_value->ref_count--;
	if (mapped_value->ref_count == 0)
		return;

	munmap(mapped_value->data, mapped_key->size);
	mapped_value->file->memory_usage -= mapped_key->size;

	free(mapped_key);
	free(mapped_value);
}

mf_handle_t mf_open(const char* pathname, size_t max_memory_usage)
{
	BEGIN_FUNCTION

	if (!pathname)
		RETURN_ERRNO(EINVAL, NULL);
	
	mapped_file_t* mf = malloc(sizeof (mapped_file_t));
	if (!mf)
		RETURN_ERRNO(ENOMEM, NULL);

	mf->fd = open(pathname, O_RDWR);
	if (mf->fd == -1)
		RETURN_FAIL(NULL);

	mf->max_memory_usage = (max_memory_usage == 0) ? 0 : max(MIN_MEMORY_USAGE, max_memory_usage);
	mf->memory_usage = 0;
	hashtable_init(&mf->chunks, CHUNKS_HASHTABLE_SIZE, mapmem_hashfunction, mapmem_comparator);

	struct stat st;
	if (fstat(mf->fd, &st) == -1)
	{
		free(mf);
		close(mf->fd);
		RETURN_FAIL(NULL);
	}
	mf->file_size = st.st_size;
	mf->page_size = (size_t) sysconf(_SC_PAGE_SIZE);
	if (mf->page_size == (size_t) -1)
		RETURN_FAIL(NULL);

	mf->data = NULL;

	RETURN(mf);
}

int mf_close(mf_handle_t mf)
{
	GET_MAPPED_FILE(-1)

	if (close(file->fd) == -1)
		RETURN_FAIL(-1);
	
	hashtable_destroy(&file->chunks, mapmem_destroy);
	free(file);

	if (file->data)
		munmap(file->data, file->size);

	RETURN(0);
}

ssize_t mf_file_size(mf_handle_t mf)
{
	GET_MAPPED_FILE(-1)
	RETURN(file->file_size);
}

mf_mapmem_t* mf_map(mf_handle_t mf, off_t offset, size_t size)
{
	GET_MAPPED_FILE(MF_MAP_FAILED)

	if (offset >= file->file_size || size == 0)
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);
	if (file->max_memory_usage != 0 && file->memory_usage + size > file->max_memory_usage)
		RETURN_ERRNO(ENOMEM, MF_MAP_FAILED);

	size_t aligned_offset = (offset / file->page_size) * file->page_size;
	size_t offset_delta = offset - aligned_offset;
	size += offset_delta;

	mapped_key_t* key = malloc(sizeof (mapped_key_t));
	if (!key)
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);
	key->size = size;
	key->offset = offset;
	
	mapped_chunk_t* chunk = hashtable_get(&file->chunks, key);

	if (chunk)
	{
		chunk->ref_count++;
		RETURN((mf_mapmem_t*) chunk);
	}
	
	file->memory_usage += size;
	void* pointer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, file->fd, aligned_offset);
	if (pointer == MAP_FAILED)
		RETURN_FAIL(MF_MAP_FAILED);

	mapped_chunk_t* mapped_chunk = malloc(sizeof (mapped_chunk_t));
	if (!mapped_chunk)
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);

	mapped_chunk->data = (void*) (&((char*) pointer)[offset_delta]);
	mapped_chunk->file = file;
	mapped_chunk->size = size;
	mapped_chunk->offset = offset;
	mapped_chunk->ref_count = 0;

	hashtable_add(&file->chunks, key, mapped_chunk);

	RETURN((mf_mapmem_t*) mapped_chunk);
}

int mf_unmap(mf_mapmem_t* mm)
{
	BEGIN_FUNCTION

	if (!mm)
		RETURN_ERRNO(EINVAL, -1);

	mapped_chunk_t* mapped_chunk = (mapped_chunk_t*) mm;
	mapped_key_t key = 
	{
		.size = mapped_chunk->size, 
		.offset = mapped_chunk->offset
	};

	if (!hashtable_remove(&mapped_chunk->file->chunks, &key, mapmem_destroy))
	{
		free(mapped_chunk);
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
	size_t new_memory_usage = file->memory_usage - file->size + size;
	if (file->max_memory_usage != 0 && new_memory_usage > file->max_memory_usage)
		RETURN_ERRNO(ENOMEM, -1);

	size_t aligned_offset = (offset / file->page_size) * file->page_size;
	size_t offset_delta = offset - aligned_offset;
	size += offset_delta;

	file->memory_usage = new_memory_usage;
	void* pointer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, file->fd, aligned_offset);
	if (pointer == MAP_FAILED)
		RETURN_FAIL(-1);

	file->data = pointer;
	file->size = size;
	file->offset = aligned_offset;

	RETURN(0);
}

ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void* buf)
{
	GET_MAPPED_FILE(-1)

	if (mf_map_internal(mf, offset, size) == -1)
		RETURN_FAIL(-1);

	memcpy(buf, &(((char*) file->data)[offset - file->offset]), size);

	if (size >= UNMAP_READ_WRITE_SIZE)
		munmap(file->data, file->size);

	RETURN(size);
}

ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void* buf)
{
	GET_MAPPED_FILE(-1)

	if (mf_map_internal(mf, offset, size) == -1)
		RETURN_FAIL(-1);

	memcpy(&(((char*) file->data)[offset - file->offset]), buf, size);

	if (size >= UNMAP_READ_WRITE_SIZE)
	{
		munmap(file->data, file->size);
		file->data = NULL;
	}

	RETURN(size);
}
