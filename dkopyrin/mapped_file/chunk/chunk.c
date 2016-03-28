#include "chunk.h"
#include "../logger/log.h"
#include <errno.h>

int chunk_init (struct chunk *ch, size_t length, uint32_t offset, int prot, int fd){
	assert(ch);
	ch -> length = length;
	ch -> offset = offset;
	ch -> addr = mmap(NULL, length, offset, prot, MAP_PRIVATE, fd, offset);
	if (ch -> addr == MAP_FAILED) {
		LOG(ERROR, "Can't mmap file in chunk, %s", strerror(errno));
		return -1;
	}
}

int chunk_finalize (struct chunk *ch) {
	if (!munmap(ch -> addr, ch -> length)) {
		LOG(ERROR, "Can't munmap file in chunk, %s", strerror(errno));
		return -1;
	}
	return 0;
}
