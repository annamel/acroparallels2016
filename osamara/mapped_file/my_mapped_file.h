#ifndef __MY_MAPPED_FILE__
#define __MY_MAPPED_FILE__
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
typedef struct {
	int offset, size;
	
} group;
typedef struct {
	off_t offset;
	size_t size;
	uint64_t refCounter;
	void* ptr, *true_ptr;
} mapmem_handle_t;
typedef struct  {
	int fd;
	long long int mapped;
	long long int fsize;
	mapmem_handle_t* r_map;
} handle_t;


#endif

