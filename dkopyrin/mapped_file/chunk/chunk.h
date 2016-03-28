#ifndef __CHUNK_H_
#define __CHUNK_H_

#include <stdlib.h>

struct chunk {
	void * addr;
	size_t length;
	size_t offset;
}

int chunk_init (struct chunk *ch, size_t length, int fd);
int chunk_finalize (struct chunk *ch);

#endif
