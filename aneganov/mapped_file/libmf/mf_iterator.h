#ifndef __MF_ITERATOR_H__
#define __MF_ITERATOR_H__

#include <sys/types.h>
#include "chunk_manager.h"

struct mf_iter {
	chpool_t *cpool;
	chunk_t *chunk;
	off_t offset;
	void *ptr;
	size_t size;
	size_t step_size;
};

int mf_iter_init(chpool_t *cpool, off_t offset, size_t size, struct mf_iter *it);

static inline int mf_iter_fini(struct mf_iter *it) {
	return it->chunk ? chunk_release(it->chunk) : 0;
}

int mf_iter_next(struct mf_iter *it);

#define mf_iter_empty(it) (!((it)->size))

#endif