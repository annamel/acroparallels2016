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
typedef struct  {
	int fd;
} my_mf_handle_t;

typedef struct {
	off_t offset;
	size_t size;
	uint64_t refCounter;
	void* ptr, *true_ptr;
} my_mf_mapmem_handle_t;
#endif

