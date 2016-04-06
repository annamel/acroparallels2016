#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <assert.h>

#include "mf_malloc.h"
#include "hashtable.h"
#include "chunk_manager.h"

struct Chunk {
	off_t idx;
	off_t len;
	unsigned ref_cnt;
	void *payload;
}; /* chunk_t */

struct pool_unit {
	chunk_t *chunk;
	struct pool_unit *next;
};

struct ChunkPool {
	int fd;
	int prot;
	size_t size;
	size_t nr_pages;
	struct pool_unit *chunk_list;
	hashtable_t *ht;
}; /* chpool_t */

static size_t get_chunk_size(off_t multiplier) {
	return multiplier * sysconf(_SC_PAGESIZE);
}

static int chunk_init(off_t idx, off_t len, int prot, int fd, chunk_t *chunk) {
	if(chunk == NULL || fd < 0)
		return EINVAL;

	chunk->idx = idx;
	chunk->len = len;
	chunk->ref_cnt = 1;
	chunk->payload = mmap(NULL, get_chunk_size(chunk->len), prot, MAP_SHARED, fd, get_chunk_size(idx));

	if(chunk->payload == MAP_FAILED)
		return errno;
	return 0;
}

static int chunk_destruct(chunk_t *chunk) {
	if(chunk == NULL || chunk->payload == NULL)
		return EINVAL;

	if( munmap(chunk->payload, chunk->len*get_chunk_size(1)) == -1 )
		return errno;

	return mf_free( sizeof(chunk_t), (void **)&chunk );
}

static bool chpool_try_to_flush(chpool_t *cpool) {
	assert(cpool);

	if(cpool->chunk_list == NULL)
		return false;

	struct pool_unit *iter = cpool->chunk_list;
	struct pool_unit *prev = NULL;
	while(iter->chunk->ref_cnt && iter->next) {
		prev = iter;
		iter = iter->next;
	}

	if(iter->chunk->ref_cnt > 0)
		return false;

	for(unsigned i = 0; i < iter->chunk->len; i++) {
		int err = hashtable_del(cpool->ht, iter->chunk->idx+i);
		assert(!err);
	}

	cpool->nr_pages -= iter->chunk->len;

	int err = chunk_destruct(iter->chunk);
	assert(!err);

	if(prev == NULL)
		cpool->chunk_list = iter->next;
	else prev->next = iter->next;

	err = mf_free(sizeof(struct pool_unit), (void **)&iter);
	assert(!err);

	return true;
}

static int chpool_add(chpool_t *cpool, chunk_t *chunk) {
	if(cpool == NULL || chunk == NULL)
		return EINVAL;

	while( cpool->nr_pages + chunk->len > cpool->size && chpool_try_to_flush(cpool) );

	if(cpool->nr_pages + chunk->len > cpool->size)
		return ENOBUFS;

	struct pool_unit *newunit = NULL;
	int err = mf_malloc(sizeof(struct pool_unit), (void **)&newunit);
	if(err) return err;

	newunit->next = NULL;
	newunit->chunk = chunk;

	if(cpool->chunk_list == NULL)
		cpool->chunk_list = newunit;
	else {
		struct pool_unit *iter = cpool->chunk_list;
		while(iter->next)
			iter = iter->next;
		iter->next = newunit;
	}

	for(unsigned i = 0; i < chunk->len; i++) {
		int err = hashtable_add(cpool->ht, chunk->idx+i, (hval_t)chunk);
		if(err) return err;
	}

	cpool->nr_pages += chunk->len;
	return 0;
}

static int chunk_construct(chpool_t *cpool, off_t idx, off_t len, chunk_t **chunk) {
	if(chunk == NULL || cpool == NULL)
		return EINVAL;

	int err = mf_malloc(sizeof(chunk_t), (void **)chunk);
	if(err) return err;

	err = chunk_init(idx, len, cpool->prot, cpool->fd, *chunk);
	if(err) return err;

	err = chpool_add(cpool, *chunk);
	if(err) return err;

	return 0;
}

static int chunk_get(chpool_t *cpool, off_t idx, off_t len, chunk_t **chunk) {
	if(cpool == NULL || chunk == NULL)
		return EINVAL;

	void *prev_val = NULL;
	*chunk = NULL;

	bool was_match = false;
	bool was_fault = false;

	for(unsigned i = 0; i < len; i++) {
		int err = hashtable_get(cpool->ht, idx+i, (hval_t *)chunk);
		if( err != 0 && err != ENOKEY )
			return err;
		if(err == 0)
			was_match = true;
		if( err == ENOKEY || (i > 0 && *chunk != prev_val) )
			was_fault = true;
		prev_val = *chunk;
	}

	if(was_match && !was_fault) {
		(*chunk)->ref_cnt++;
		return 0;
	}
	else return ENOKEY;
}

int chunk_acquire(chpool_t *cpool,  off_t offset, size_t size, chunk_t **chunk) {
	if(cpool == NULL || chunk == NULL || cpool->fd < 0 || cpool->nr_pages > cpool->size)
		return EINVAL;

	if(size == 0) {
		*chunk = NULL;
		return 0;
	}

	off_t idx = offset / get_chunk_size(1);
	off_t len = size/get_chunk_size(1) + 1;

	int err = chunk_get(cpool, idx, len, chunk);

	if(err == ENOKEY) {
		chunk_construct(cpool, idx, len, chunk);
		return 0;
	}
	else return err; /* 0 -- success -- is in this case */
}

int chunk_find(chpool_t *cpool, off_t offset, size_t size, chunk_t **chunk) {
	if(cpool == NULL || chunk == NULL || cpool->fd < 0 || cpool->nr_pages > cpool->size)
		return EINVAL;

	if(size == 0) {
		*chunk = NULL;
		return 0;
	}

	off_t idx = offset / get_chunk_size(1);
	off_t len = size / get_chunk_size(1) + 1;

	return chunk_get(cpool, idx, len, chunk);
}

int chunk_release(chunk_t *chunk) {
	if(chunk == NULL)
		return EINVAL;

	if(chunk->ref_cnt == 0)
		return EAGAIN;

	chunk->ref_cnt--;
	return 0;
}

static int off_cmp(hkey_t key1, hkey_t key2) {
	return (key1 > key2) ? 1 : (key1 < key2) ? -1 : 0;
}

static int chpool_init(chpool_t *cpool, unsigned size, int fd, int prot) {
	cpool->chunk_list = NULL;
	cpool->size = size;
	cpool->nr_pages = 0;
	cpool->fd = fd;
	cpool->prot = prot;
	cpool->ht = NULL;
	return hashtable_construct(off_cmp, simple_hash, &cpool->ht);
}

int chpool_construct(size_t max_mem, int fd, int prot, chpool_t **cpool) {
	size_t size = max_mem / get_chunk_size(1);

	int err = mf_malloc( sizeof(chpool_t), (void **)cpool );
	if(err) return err;

	return chpool_init(*cpool, size, fd, prot);
}

static int chpool_fini(chpool_t *cpool) {
	if(cpool == NULL)
		return EINVAL;

	while(cpool->chunk_list) {
		struct pool_unit *iter = cpool->chunk_list;
		cpool->chunk_list = iter->next;

		int err = chunk_destruct(iter->chunk);
		if(err) return err;

		err = mf_free(sizeof(struct pool_unit), (void **)&iter);
		if(err) return err;
	}

	cpool->nr_pages = 0;

	int err = hashtable_destruct(&cpool->ht);
	if(err) return err;

	err = close(cpool->fd);
	if(err == -1) return errno;

	return 0;
}
int chpool_destruct(chpool_t **cpool) {
	if(cpool == NULL)
		return EINVAL;

	int err = chpool_fini(*cpool);
	if(err) return err;

	err = mf_free( sizeof(chpool_t), (void **)cpool );
	if(err) return err;

	return 0;
}

int chunk_get_mem(chunk_t *chunk, off_t offset, void **buf) {
	if(chunk == NULL || buf == NULL)
		return EINVAL;

	off_t left = get_chunk_size(chunk->idx);
	off_t right = left + get_chunk_size(chunk->len);
	
	if(offset < left || offset > right)
		return EINVAL;

	off_t choff = offset - left;
	*buf = chunk->payload + choff;

	return 0;
}