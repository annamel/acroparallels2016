#include "mfdef.h"
#include "mf_iterator.h"

#define min(x,y) \
	( (x < y) ? x : y)

static inline size_t mf_iter_step_size(void) {
	return (size_t)sysconf(_SC_PAGESIZE);
}

static int mf_iter_step(struct mf_iter *it) {
	size_t page_size = mf_iter_step_size();
	if(it->chunk) {
		int err = chunk_release(it->chunk);
		if(unlikely(err)) return err;
	}
	int err = chunk_find(it->cpool, it->choff, page_size - 1,  &it->chunk);
	switch(err) {
		case 0:
			return chunk_get_mem(it->chunk, it->offset, &it->ptr);
			break;
		case ENOKEY:
			it->chunk = NULL;
			it->ptr = NULL;
			return 0;
			break;
		default:
			return err;
			break;
	}
}

int mf_iter_init(chpool_t *cpool, off_t offset, size_t size, struct mf_iter *it) {
	it->cpool = cpool;
	it->size = size;
	it->offset = offset;
	size_t page_size = mf_iter_step_size();
	it->choff = offset - (offset % page_size);
	it->step_size = min(size, mf_iter_step_size() - (offset - it->choff));
	return mf_iter_step(it);
}

int mf_iter_next(struct mf_iter *it) {
	it->size -= it->step_size;
	size_t page_size = mf_iter_step_size();
	it->step_size = min(it->size, page_size);
	it->offset = it->choff += page_size;
	return mf_iter_step(it);
}