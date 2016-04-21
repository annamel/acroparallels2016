#include "chunk.h"

#define COLOR(x) "\x1B[34m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"

#include <errno.h>
#include <assert.h>
#include <sys/mman.h>
#include <string.h>

int chunk_init_unused (struct chunk *ch) {
	assert(ch);
	ch -> state = -1;
	return 0;
}

int chunk_init (struct chunk *ch, size_t length, long int offset, int prot, int fd){
	LOG(INFO, "Chunk init called\n");
	assert(ch);
	ch -> length = length;
	ch -> offset = offset;
	ch -> addr = mmap(NULL, length, prot, MAP_SHARED, fd, offset);
	ch -> state++;
	ch -> ref_cnt = 0;
	if (ch -> addr == MAP_FAILED) {
		LOG(ERROR, "Can't mmap file in chunk, %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int chunk_finalize (struct chunk *ch) {
	LOG(INFO, "Chunk finalize called\n");
	if (msync(ch -> addr, ch -> length, MS_SYNC) == -1) {
		LOG(ERROR, "Can't sync file, %s\n", strerror(errno));
		return -1;
	}

	if (munmap(ch -> addr, ch -> length)) {
		LOG(ERROR, "Can't munmap file in chunk, %s\n", strerror(errno));
		return -2;
	}
	return 0;
}

void *chunk_cpy_c2b(void *buf, struct chunk *ch, size_t num, long int offset){
	LOG(DEBUG, "c2b by offset %d %d bytes\n", offset, num);
	//TODO: Clever checks for overflow
	return memcpy(buf, ch -> addr + offset, num);
}
void *chunk_cpy_b2c(struct chunk *ch, const void *buf, size_t num, long int offset){
	LOG(DEBUG, "b2c by offset %d %d bytes\n", offset, num);
  	//TODO: Clever checks for overflow
	return memcpy(ch -> addr + offset, buf, num);
}

void *chunk_remap(struct chunk *ch, size_t new_length){
	void *ret = mremap(ch -> addr, ch -> length, new_length, 0);
	if (ret == MAP_FAILED) return ret;
	ch -> length = new_length;

	return ret;
}
