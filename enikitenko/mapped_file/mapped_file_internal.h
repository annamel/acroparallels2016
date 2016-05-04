#ifndef __MAPPED_FILE_INTERNAL__
#define __MAPPED_FILE_INTERNAL__

#include <mapped_file.h>
#include "../logger/log.h"
#include "hashtable.h"

typedef struct
{
	size_t size;
	off_t offset;

} mapped_chunk_key_t;

struct mapped_file;

typedef struct mapped_chunk
{
	void* data;
	size_t size;
	off_t offset;
	int ref_count;
	struct mapped_chunk* next;

} mapped_chunk_t;

typedef struct mapped_file
{
	int fd;
	size_t page_size;
	size_t chunk_size;
	size_t file_size;
	hashtable_t chunks;

	mapped_chunk_t* free_chunks;
	int mapped_memory_usage;

	int fully_mapped;
	void* data;
	size_t size;
	off_t offset;

} mapped_file_t;

#define CHUNKS_HASHTABLE_SIZE 40009
#define READ_WRITE_MIN_SIZE (4*1024*1024)
#define UNMAP_READ_WRITE_SIZE (64*1024*1024)
#define MAP_CHUNK_SIZE (512*1024*1024)

#endif // __MAPPED_FILE_INTERNAL__
