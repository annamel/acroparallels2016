#define _GNU_SOURCE 1
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <string.h>

#include "mf_malloc.h"
#include "hashtable.h"
#include "log.h"
#include "bug.h"
#include "chunk_manager.h"

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
	chunk_t *head;
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
	chunk->prev = chunk;
	chunk->next = chunk;
	chunk->cpool = cpool;
	chunk->payload = mmap(NULL, get_chunk_size(chunk->len), cpool->prot, MAP_SHARED, cpool->fd, get_chunk_size(idx));
	log_write(LOG_DEBUG, "chunk_init: offset = %jd, chunk->payload == %p\n", get_chunk_size(idx), chunk->payload);

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
	log_write(LOG_INFO, "trying to delete chunk with null reference counter from chpool...\n");
	log_write(LOG_DEBUG, "chpool_del: chunk = %p, chunk->ref_cnt = %u\n", chunk, chunk ? chunk->ref_cnt : -1);

	if(chunk == NULL || chunk->ref_cnt != 0)
		return EBUSY;

	log_write(LOG_DEBUG, "chpool_del: chunk->prev = %p, chunk->next = %p\n", chunk->prev, chunk->next);
	if(chunk != chunk->next) {
		chunk->prev->next = chunk->next;
		chunk->next->prev = chunk->prev;
		if(chunk == chunk->cpool->head)
			chunk->cpool->head = chunk->next;	
	}
	else chunk->cpool->head = NULL;

	for(unsigned i = 0; i < chunk->len; i++) {
		int err = hashtable_del(chunk->cpool->ht, chunk->idx+i);
		if(err && err != ENOKEY) return err;
	}

	chunk->cpool->nr_pages -= chunk->len;

	int err = chunk_destruct(chunk);
	if(err) return err;

	return 0;
}

static int chpool_add(chunk_t *chunk) {
	log_write(LOG_INFO, "adding new chunk to chpool...\n");
	chpool_t *cpool = chunk->cpool;

	int err = 0;
	while( cpool->nr_pages + chunk->len > cpool->size && !(err = chpool_del(cpool->head)) );

	if(err == EBUSY) return ENOBUFS;
	if(err) return err;

	log_write(LOG_INFO, "chpool is ready to be enlarged...\n");

/* New chunks must be stored at the end of the list */
	if(cpool->head != NULL) {
		chunk->next = cpool->head;
		chunk->prev = cpool->head->prev;
		chunk->next->prev = chunk;
		chunk->prev->next = chunk;
	}
	else cpool->head = chunk;

	log_write(LOG_DEBUG, "idx = %jd, len = %jd\n", chunk->idx, chunk->len);

	for(unsigned i = 0; i < chunk->len; i++) {
		int err = hashtable_add(cpool->ht, chunk->idx+i, (hval_t)chunk);
		if(err) return err;
	}

	cpool->nr_pages += chunk->len;

	log_write(LOG_INFO, "chpool_add finished\n");

	return 0;
}

static int chunk_construct(chpool_t *cpool, off_t idx, off_t len, chunk_t **chunk) {
	int err = mf_malloc(sizeof(chunk_t), (void **)chunk);
	if(err) return err;

	err = chunk_init(idx, len, cpool, *chunk);
	if(err) return err;

	err = chpool_add(*chunk);
	if(err) return err;

	log_write(LOG_INFO, "chunk_construct(idx=%jd, len=%jd): success\n", idx, len);
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

int chunk_acquire(chpool_t *cpool,  off_t offset, size_t size, chunk_t **chunk_ptr) {
	if(!cpool || !chunk_ptr) return EINVAL;

	BUG_ON(!cpool->ht || cpool->fd < 0 || cpool->nr_pages > cpool->size || !cpool->size);

	if(size == 0) {
		*chunk_ptr = NULL;
		return 0;
	}

	off_t idx = offset / get_chunk_size(1);
	off_t len = size/get_chunk_size(1) + 1;

	int err = chunk_get(cpool, idx, len, chunk_ptr);
	log_write(LOG_INFO, "chunk_acquire: chunk_get(idx=%jd, len=%jd): %s\n", idx, len, strerror(err));

	chunk_t *chunk = *chunk_ptr;

	switch(err) {
		case 0:
			chunk->ref_cnt++;
			return 0;
			break;
		case ENOKEY:
			if( *chunk_ptr == NULL || !(err = chpool_del(chunk)) )
				return chunk_construct(cpool, idx, len, chunk_ptr);
			if(err != EBUSY) return err;
			void *newpayload = mremap(chunk->payload, get_chunk_size(chunk->len), get_chunk_size(idx + len - chunk->idx), 0);
			if(newpayload != MAP_FAILED) {
				chunk->payload = newpayload;
				chunk->ref_cnt++;
				return 0;
			}
			else {
				if(errno != ENOMEM) return errno;
				log_write(LOG_INFO, "mremap failed\n");
				return chunk_construct(cpool, idx, len, chunk_ptr);
			}
			break;
		default:
			return err;
			break;
	}
}

int chunk_find(chpool_t *cpool, off_t offset, size_t size, chunk_t **chunk) {
	if(!cpool || !chunk) return EINVAL;

	BUG_ON(!cpool->ht || cpool->fd < 0 || cpool->nr_pages > cpool->size || !cpool->size);

	if(size == 0) {
		*chunk = NULL;
		return 0;
	}

	off_t idx = offset / get_chunk_size(1);
	off_t len = size / get_chunk_size(1) + 1;

	int err = chunk_get(cpool, idx, len, chunk);
	if(err) return err;
	(*chunk)->ref_cnt++;
	return 0;
}

int chunk_release(chunk_t *chunk) {
	if(!chunk) return EINVAL;

	if(chunk->ref_cnt == 0) return EAGAIN;

/* Chunks with ref_cnt == 0 must be stored at the beginning of the list */
	if(--chunk->ref_cnt == 0 && chunk != chunk->cpool->head) {
		chunk->prev->next = chunk->next;
		chunk->next->prev = chunk->prev;

		chunk_t *oldhead = chunk->cpool->head;

		chunk->next = oldhead;
		chunk->prev = oldhead->prev;

		chunk->next->prev = chunk;
		chunk->prev->next = chunk;

		chunk->cpool->head = chunk;
	}

	log_write(LOG_INFO, "chunk_release: for chunk %p ref_cnt == %u\n", chunk, chunk->ref_cnt);

	return 0;
}

static int off_cmp(hkey_t key1, hkey_t key2) {
	return (key1 > key2) ? 1 : (key1 < key2) ? -1 : 0;
}

static int chpool_init(chpool_t *cpool, unsigned size, int fd, int prot) {
	cpool->head = NULL;
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
	if(fd < 0 || cpool == NULL || ( !(prot & PROT_READ) && !(prot & PROT_WRITE) ) )
		return EINVAL;

	BUILD_BUG_ON(DEFAULT_CHPOOL_SIZE == 0);
	size_t size = max_mem ? max_mem / get_chunk_size(1) : DEFAULT_CHPOOL_SIZE;
	if(size == 0) return EINVAL;

	int err = mf_malloc( sizeof(chpool_t), (void **)cpool );
	if(err) return err;

	return chpool_init(*cpool, size, fd, prot);
}

static int chpool_fini(chpool_t *cpool) {
	log_write(LOG_INFO, "chpool finalizing...\n");

	if(cpool->head){
		cpool->head->prev->next = NULL;
		cpool->head->prev = NULL;
	}

	while(cpool->head) {
		log_write(LOG_DEBUG, "head = %p\n", cpool->head);
		chunk_t *iter = cpool->head;
		cpool->head = iter->next;

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
	if(cpool == NULL) return EINVAL;

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

	log_write(LOG_DEBUG, "chunk_get_mem: chunk->payload == %p, *buf = %p\n", chunk->payload, *buf);

	return 0;
}

int chpool_fd(chpool_t *cpool) {
	if(!cpool || cpool->fd < 0)
		return -1;
	return cpool->fd;
}

int chpool_mem_add(void *ptr, chunk_t *chunk) {
	return hashtable_add(chunk->cpool->mem_ht, (hkey_t)ptr, (hval_t)chunk);
}

int chpool_mem_get(chpool_t *cpool, void *ptr, chunk_t **chunk) {
	return hashtable_get(cpool->mem_ht, (hkey_t)ptr, (hval_t *)chunk);
}