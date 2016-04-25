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

static int mapmem_destroy(void* key, void* value)
{
	mapped_key_t* mapped_key = (mapped_key_t*) key;
	mapped_chunk_t* mapped_value = (mapped_chunk_t*) value;

	mapped_value->ref_count--;
	if (mapped_value->ref_count != 0)
		return 0;

	munmap(mapped_value->data, mapped_key->size);

	free(mapped_key);
	free(mapped_value);

	return 1;
}

mf_handle_t mf_open(const char* pathname)
{
	BEGIN_FUNCTION

	if (!pathname)
		RETURN_ERRNO(EINVAL, MF_OPEN_FAILED);
	
	mapped_file_t* mf = malloc(sizeof (mapped_file_t));
	if (!mf)
		RETURN_ERRNO(ENOMEM, MF_OPEN_FAILED);

	mf->fd = open(pathname, O_RDWR);
	if (mf->fd == -1)
		RETURN_FAIL(MF_OPEN_FAILED);

	hashtable_init(&mf->chunks, CHUNKS_HASHTABLE_SIZE, mapmem_hashfunction, mapmem_comparator);

	struct stat st;
	if (fstat(mf->fd, &st) == -1)
	{
		free(mf);
		close(mf->fd);
		RETURN_FAIL(MF_OPEN_FAILED);
	}
	mf->file_size = st.st_size;
	mf->page_size = (size_t) sysconf(_SC_PAGE_SIZE);
	if (mf->page_size == (size_t) -1)
		RETURN_FAIL(MF_OPEN_FAILED);

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

	mapped_key_t key = 
	{
		.size = size, 
		.offset = offset
	};
	mapped_chunk_t* chunk = hashtable_get(&file->chunks, &key);

	if (chunk)
	{
		chunk->ref_count++;
		*mapmem_handle = (mf_mapmem_handle_t) chunk;
		RETURN(chunk->data);
	}

	mapped_key_t* key_ptr = malloc(sizeof (mapped_key_t));
	if (!key_ptr)
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);
	key_ptr->size = size;
	key_ptr->offset = offset;

	void* pointer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, file->fd, aligned_offset);
	if (pointer == MAP_FAILED)
		RETURN_FAIL(MF_MAP_FAILED);

	chunk = malloc(sizeof (mapped_chunk_t));
	if (!chunk)
		RETURN_ERRNO(EINVAL, MF_MAP_FAILED);

	chunk->data = (void*) (&((char*) pointer)[offset_delta]);
	chunk->size = size;
	chunk->offset = offset;
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

	mapped_chunk_t* mapped_chunk = (mapped_chunk_t*) mapmem_handle;
	mapped_key_t key = 
	{
		.size = mapped_chunk->size, 
		.offset = mapped_chunk->offset
	};

	if (!hashtable_remove(&file->chunks, &key, mapmem_destroy))
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

	memcpy(&(((char*) file->data)[offset - file->offset]), buf, count);

	if (count >= UNMAP_READ_WRITE_SIZE)
	{
		munmap(file->data, file->size);
		file->data = NULL;
	}

	RETURN(count);
}
