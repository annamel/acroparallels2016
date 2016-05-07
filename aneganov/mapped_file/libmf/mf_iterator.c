#include <stddef.h>
#include <errno.h>

#include "mfdef.h"
#include "chunk_manager.h"
#include "log.h"
#include "mf_iterator.h"

struct mf_iter_private {
	chpool_t *cpool;
	chunk_t  *chunk;
	size_t size;
} __attribute__ ((__packed__));

static int mf_iter_step(struct mf_iter *it) {
	struct mf_iter_private *it_private = (struct mf_iter_private *)it->private;
	off_t border;
	size_t page_size = chpool_page_size(it_private->cpool);

	chunk_release(it_private->chunk);

	int err = chunk_find(it_private->cpool, it->offset, page_size - 1,  &it_private->chunk);
	switch(err) {
		case 0:
			err = chunk_get_mem(it_private->chunk, it->offset, &it->ptr, &border);
			if(unlikely(err)) {
				return err;
			}
			it->step_size = min(it_private->size, border - it->offset);
			break;
		case ENOKEY:
			it_private->chunk = NULL;
			it->ptr = NULL;
			it->step_size = min(it_private->size, page_size - (it->offset % page_size));
			break;
		default:
			it_private->chunk = NULL;
			it->ptr = NULL;
			return err;
			break;
	}
	return 0;
}

int mf_iter_init(chpool_t *cpool, off_t offset, size_t size, struct mf_iter *it) {
	if(!cpool || !it || offset < 0) {
		return EINVAL;
	}
	struct mf_iter_private *it_private = (struct mf_iter_private *)it->private;
	it_private->cpool = cpool;
	it_private->chunk = NULL;
	it_private->size = size;
	it->offset = offset;
	return mf_iter_step(it);
}

int mf_iter_next(struct mf_iter *it) {
#ifdef DEBUG
	if(!mf_iter_is_valid(it)) {
		return EINVAL;
	}
#endif
	struct mf_iter_private *it_private = (struct mf_iter_private *)it->private;
	it->offset += it->step_size;
	it_private->size -= it->step_size;
	return mf_iter_step(it);
}

int mf_iter_empty(struct mf_iter *it) {
#ifdef DEBUG
	if(!mf_iter_is_valid(it)) {
		return EINVAL;
	}
#endif
	struct mf_iter_private *it_private = (struct mf_iter_private *)it->private;
	return !(it_private->size);
}

int mf_iter_fini(struct mf_iter *it) {
#ifdef DEBUG
	if(!mf_iter_is_valid(it)) {
		return EINVAL;
	}
#endif
	struct mf_iter_private *it_private = (struct mf_iter_private *)it->private;
	return it_private->chunk ? chunk_release(it_private->chunk) : 0;
}

bool mf_iter_is_valid(struct mf_iter *it) {
	if(it == NULL || it->offset < 0) {
		return false;
	}

	struct mf_iter_private *it_private = (struct mf_iter_private *)it->private;
	if( it_private->cpool == NULL || (it->step_size == 0 && it_private->size != 0) || (it->step_size > it_private->size) ){
		return false;
	}

	return true;
}