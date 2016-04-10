#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <assert.h>

#include "mf_malloc.h"
#include "hashtable.h"
#include "chunk_manager.h"
#include "log.h"

#define DEFAULT_CHPOOL_SIZE (0x40000)

struct Chunk {
	chpool_t *cpool;
	off_t idx;
	off_t len;
	unsigned ref_cnt;
	chunk_t *next;
	chunk_t *prev;
	void *payload;
}; /* chunk_t */

struct ChunkPool {
	int fd;
	int prot;
	size_t size;
	size_t nr_pages;
	chunk_t *chunk_list;
	hashtable_t *ht;
	hashtable_t *mem_ht;
}; /* chpool_t */

static size_t get_chunk_size(off_t multiplier) {
	return multiplier * sysconf(_SC_PAGESIZE);
}

static int chunk_init(off_t idx, off_t len, chpool_t *cpool, chunk_t *chunk) {
	chunk->idx = idx;
	chunk->len = len;
	chunk->ref_cnt = 1;
	chunk->prev = NULL;
	chunk->next = NULL;
	chunk->cpool = cpool;
	chunk->payload = mmap(NULL, get_chunk_size(chunk->len), cpool->prot, MAP_SHARED, cpool->fd, get_chunk_size(idx));

	if(chunk->payload == MAP_FAILED)
		return errno;
	return 0;
}

static int chunk_destruct(chunk_t *chunk) {
	if( munmap(chunk->payload, chunk->len*get_chunk_size(1)) == -1 )
		return errno;

	return mf_free( sizeof(chunk_t), (void **)&chunk );
}

static int chpool_del(chunk_t *chunk) {
	chpool_t *cpool = chunk->cpool;

	if(chunk == NULL || chunk->ref_cnt != 0)
		return EBUSY;

	if(chunk->prev) chunk->prev->next = chunk->next;
	if(chunk->next) chunk->next->prev = chunk->prev;

	for(unsigned i = 0; i < chunk->len; i++) {
		int err = hashtable_del(cpool->ht, chunk->idx+i);
		if(err && err != ENOKEY) return err;
	}

	cpool->nr_pages -= chunk->len;

	int err = chunk_destruct(chunk);
	if(err) return err;

	return 0;
}

static int chpool_add(chunk_t *chunk) {
	chpool_t *cpool = chunk->cpool;

	int err = 0;
	while( cpool->nr_pages + chunk->len > cpool->size && !(err = chpool_del(cpool->chunk_list)) );

	if(err == EBUSY) return ENOBUFS;
	if(err) return err;

	chunk->next = cpool->chunk_list;
	chunk->prev = NULL;
	cpool->chunk_list = chunk;

	for(unsigned i = 0; i < chunk->len; i++) {
		int err = hashtable_add(cpool->ht, chunk->idx+i, (hval_t)chunk);
		if(err) return err;
	}

	cpool->nr_pages += chunk->len;
	return 0;
}

static int chunk_construct(chpool_t *cpool, off_t idx, off_t len, chunk_t **chunk) {
	int err = mf_malloc(sizeof(chunk_t), (void **)chunk);
	if(err) return err;

	err = chunk_init(idx, len, cpool, *chunk);
	if(err) return err;

	err = chpool_add(*chunk);
	if(err) return err;

	return 0;
}

static int chunk_get(chpool_t *cpool, off_t idx, off_t len, chunk_t **chunk) {
	*chunk = NULL;

	int err = hashtable_get(cpool->ht, idx, (hval_t *)chunk);
	if(err) return err;

	if( (*chunk)->idx + (*chunk)->len < idx + len )
		return ENOKEY;

	return 0;
}

int chunk_acquire(chpool_t *cpool,  off_t offset, size_t size, chunk_t **chunk) {
	if(!cpool || !chunk || !cpool->ht || cpool->fd < 0 || cpool->nr_pages > cpool->size)
		return EINVAL;

	if(size == 0) {
		*chunk = NULL;
		return 0;
	}

	off_t idx = offset / get_chunk_size(1);
	off_t len = size/get_chunk_size(1) + 1;

	int err = chunk_get(cpool, idx, len, chunk);

	if(err == ENOKEY) {
		if( *chunk != NULL ) {
			err = chpool_del(*chunk);
			if(err && err != EBUSY) return err;
		}
		chunk_construct(cpool, idx, len, chunk);
		return 0;
	}
	else return err; /* 0 -- success -- in this case */
}

int chunk_find(chpool_t *cpool, off_t offset, size_t size, chunk_t **chunk) {
	if(!cpool || !chunk || !cpool->ht || cpool->fd < 0 || cpool->nr_pages > cpool->size)
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
	if(!chunk)
		return EINVAL;

	if(chunk->ref_cnt == 0)
		return EAGAIN;

	if(--chunk->ref_cnt == 0) {
		if(chunk->prev) chunk->prev->next = chunk->next;
		if(chunk->next) chunk->next->prev = chunk->prev;
		chunk->next = chunk->cpool->chunk_list;
		chunk->prev = NULL;	
	}

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
	int err =  hashtable_construct(off_cmp, simple_hash, &cpool->ht);
	if(err) return err;
	return hashtable_construct(off_cmp, simple_hash, &cpool->mem_ht);
}

int chpool_construct(size_t max_mem, int fd, int prot, chpool_t **cpool) {
	size_t size = max_mem ? max_mem / get_chunk_size(1) : DEFAULT_CHPOOL_SIZE;

	int err = mf_malloc( sizeof(chpool_t), (void **)cpool );
	if(err) return err;

	return chpool_init(*cpool, size, fd, prot);
}

static int chpool_fini(chpool_t *cpool) {
	while(cpool->chunk_list) {
		chunk_t *iter = cpool->chunk_list;
		cpool->chunk_list = iter->next;

		int err = chunk_destruct(iter);
		if(err) return err;
	}

	cpool->nr_pages = 0;

	int err = hashtable_destruct(&cpool->ht);
	if(err) return err;

	err = hashtable_destruct(&cpool->mem_ht);
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

int chpool_fd(chpool_t *cpool) {
	if(!cpool || !cpool->ht || cpool->fd < 0 || cpool->nr_pages > cpool->size)
		return -1;
	return cpool->fd;
}

int chpool_mem_add(void *ptr, chunk_t *chunk) {
	return hashtable_add(chunk->cpool->mem_ht, (hkey_t)ptr, (hval_t)chunk);
}

int chpool_mem_get(chpool_t *cpool, void *ptr, chunk_t **chunk) {
	return hashtable_get(cpool->mem_ht, (hkey_t)ptr, (hval_t *)chunk);
}
