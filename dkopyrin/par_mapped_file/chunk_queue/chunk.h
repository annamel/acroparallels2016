#ifndef __CHUNK_H_
#define __CHUNK_H_

#include <stdlib.h>

struct chunk {
	void * addr;
	size_t length;
	off_t offset;
	struct chunk* next;
	int ref_cnt;
};

int chunk_init (struct chunk *ch, size_t length, off_t offset, int fd);
int chunk_init_unused (struct chunk *ch);
int chunk_finalize (struct chunk *ch);

void *chunk_cpy_c2b(void *buf, struct chunk *ch, size_t num, off_t offset);
void *chunk_cpy_b2c(struct chunk *ch, const void *buf, size_t num, off_t offset);

#endif
