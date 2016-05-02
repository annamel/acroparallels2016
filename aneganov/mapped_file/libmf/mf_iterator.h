#ifndef __MF_ITERATOR_H__
#define __MF_ITERATOR_H__

#include <sys/types.h>
#include <stdbool.h>

struct mf_iter {
	off_t offset;
	void *ptr;
	size_t step_size;
	char private[3*sizeof(void*)];
};

int mf_iter_init(chpool_t *cpool, off_t offset, size_t size, struct mf_iter *it);
int mf_iter_next(struct mf_iter *it);
int mf_iter_fini(struct mf_iter *it);
int mf_iter_empty(struct mf_iter *it);
bool mf_iter_is_valid(struct mf_iter *it);

#endif