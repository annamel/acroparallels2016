#include "chunk.h"
#include <sys/mman.h>

#define COLOR(x) "\x1B[34m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"

#include <errno.h>
#include <assert.h>
#include <string.h>

int chunk_init_unused (struct chunk *ch) {
	assert(ch);
	ch -> ref_cnt = -1;
	return 0;
}

int chunk_init (struct chunk *ch, size_t length, off_t offset, int fd){
	LOG(INFO, "Chunk init called\n");
	assert(ch);
#ifdef MEMORY_DEBUG
	/* In order to make memory test for mmap wrap mmap is created.
	 * Wrap is bigger for 2 pages than default map: 1 page for end and start.
	 * Firstly wrap is created and we know that contigous mmap memory is available.
	 * That's why we only have to mmap actual data with MAP_FIXED to make
	 * page with junk on at begin and end of mmap.
	 */
	LOG(DEBUG, "Creating wrap\n", strerror(errno));
	void *wrap = mmap(NULL, length + 2 * sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE,
			  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (wrap == MAP_FAILED) {
		LOG(ERROR, "Can't mmap wrap, %s\n", strerror(errno));
		return -1;
	}
	memset(wrap, 0x66, length + 2 * sysconf(_SC_PAGESIZE));
	ch -> addr = mmap(wrap + sysconf(_SC_PAGESIZE), length, prot,
			  MAP_SHARED | MAP_FIXED, fd, offset);
	if (ch -> addr == MAP_FAILED) {
		LOG(ERROR, "Can't mmap file in chunk with wrap, %s\n", strerror(errno));
		return -1;
	}
#else
	ch -> addr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
	if (ch -> addr == MAP_FAILED) {
		LOG(ERROR, "Can't mmap file in chunk, %s\n", strerror(errno));
		return -1;
	}
#endif

	ch -> ref_cnt = 0;
	ch -> length = length;
	ch -> offset = offset;
	return 0;
}

int chunk_finalize (struct chunk *ch) {
	LOG(INFO, "Chunk finalize called\n");
	assert(ch);

#ifdef MEMORY_DEBUG
	if (munmap(ch -> addr  - sysconf(_SC_PAGESIZE), ch -> length  + 2*sysconf(_SC_PAGESIZE))) {
		LOG(ERROR, "Can't munmap file in chunk, %s\n", strerror(errno));
		return -2;
	}
	ch -> addr = (void *)0xDEADBEEF;
#else
	if (munmap(ch -> addr, ch -> length)) {
		LOG(ERROR, "Can't munmap file in chunk, %s\n", strerror(errno));
		return -2;
	}
#endif
	return 0;
}

void *chunk_cpy_c2b(void *buf, struct chunk *ch, size_t num, off_t offset){
	LOG(DEBUG, "c2b by offset %d %d bytes\n", offset, num);
	return memcpy(buf, ch -> addr + offset, num);
}
void *chunk_cpy_b2c(struct chunk *ch, const void *buf, size_t num, off_t offset){
	LOG(DEBUG, "b2c by offset %d %d bytes\n", offset, num);
	return memcpy(ch -> addr + offset, buf, num);
}
