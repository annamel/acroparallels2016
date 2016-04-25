#ifndef __MAPPED_FILE_INTERNAL__
#define __MAPPED_FILE_INTERNAL__

#include <mapped_file.h>
#include "../logger/log.h"
#include "hashtable.h"

typedef struct
{
	size_t size;
	off_t offset;

} mapped_key_t;

struct mapped_file;

typedef struct
{
	void* data;
	size_t size;
	off_t offset;
	int ref_count;

} mapped_chunk_t;

typedef struct mapped_file
{
	int fd;
	size_t page_size;
	size_t file_size;
	hashtable_t chunks; // hashtable is useless for this task so fuck performance and memory use

	void* data;
	size_t size;
	off_t offset;

} mapped_file_t;

#define CHUNKS_HASHTABLE_SIZE 4095
#define READ_WRITE_MIN_SIZE (4096*1024)
#define MIN_MEMORY_USAGE (64*1024*1024)
#define UNMAP_READ_WRITE_SIZE (16*1024*1024)

#endif // __MAPPED_FILE_INTERNAL__
