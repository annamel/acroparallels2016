#ifndef __MAPPED_FILE_INTERNAL__
#define __MAPPED_FILE_INTERNAL__

#include <mapped_file.h>
#include <pthread.h>
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
	struct mapped_chunk* prev;
	struct mapped_chunk* next;

} mapped_chunk_t;

typedef struct mapped_file
{
	int fd;
	size_t page_size;
	size_t chunk_size;
	size_t file_size;
	int fully_mapped;

	hashtable_t chunks;
	mapped_chunk_t* free_chunks;

	void* data;
	size_t size;
	off_t offset;

#ifdef MULTITHREADING
	pthread_rwlock_t rwlock_map;
	pthread_rwlock_t rwlock_readwrite;
#endif // MULTITHREADING

} mapped_file_t;

#define CHUNKS_HASHTABLE_SIZE 40009
#define READ_WRITE_MIN_SIZE (4*1024*1024)
#define UNMAP_READ_WRITE_SIZE (64*1024*1024)

#define MAP_CHUNK_SIZE_BITS 29
#define MAP_CHUNK_SIZE (1 << MAP_CHUNK_SIZE_BITS)

#define HASHTABLE_SIZE_BITS (CHAR_BIT * sizeof (uint64_t) / 2)
#define HASHTABLE_OFFSET_BITS HASHTABLE_OFFSET_BITS
#define MAX_CHUNK_SIZE (1 << (MAP_CHUNK_SIZE + HASHTABLE_SIZE_BITS))

#define IS_POWER_OF_2(x) ((x) && !((x) & ((x) - 1)))
#define TO_KEY(offset,size) (((offset) >> MAP_CHUNK_SIZE_BITS) << HASHTABLE_SIZE_BITS + ((size) >> MAP_CHUNK_SIZE_BITS))

#endif // __MAPPED_FILE_INTERNAL__
