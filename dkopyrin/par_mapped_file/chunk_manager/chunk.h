#ifndef __CHUNK_H_
#define __CHUNK_H_

#include <stdlib.h>

#define STATE_OK 0
#define STATE_BEING_FREED 0

union tagged_ref_cnt{
	int64_t trc;
	struct{
		int32_t state;
		int32_t ref_cnt;
	};
};

struct chunk {
	size_t length;
	off_t offset;
	void * addr;
	union tagged_ref_cnt trc;
};

int chunk_init (struct chunk *ch, size_t length, off_t offset, int fd);
int chunk_init_unused (struct chunk *ch);
int chunk_finalize (struct chunk *ch);

void *chunk_cpy_c2b(void *buf, struct chunk *ch, size_t num, off_t offset);
void *chunk_cpy_b2c(struct chunk *ch, const void *buf, size_t num, off_t offset);

#endif
