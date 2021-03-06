#define _GNU_SOURCE 1
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "list.h"
#include "mfdef.h"
#include "mf_malloc.h"
#include "map.h"
#include "log.h"
#include "bug.h"
#include "chunk.h"
#include "chunk_manager.h"

struct chunk_pool {
	int fd;
	off_t fsize;
	int prot;
	size_t threshold;
	size_t nr_pages;
	size_t pg_sz;
	unsigned pg_order;
	off_t pg_mask;
	list_t head;
	map_t *map;
}; /* chpool_t */

static int chunk_destruct(chunk_t *chunk) {
	if( munmap(chunk->payload, chunk->key.len * chunk->cpool->pg_sz) == -1 ) {
		return errno;
	}

	mf_free(chunk);
	return 0;
}

static int chpool_del(list_t *pos) {
	if(list_empty(pos)) {
		return EBUSY;
	}

	chunk_t *chunk = list_entry(pos, chunk_t, list);

	if(chunk == NULL || chunk->ref_cnt != 0) {
		return EBUSY;
	}

	list_del(&chunk->list);

	int err = map_del(chunk->cpool->map, &chunk->key, chunk->is_indexed);
	if(unlikely(err && err != ENOKEY))
		return err;

	chunk->cpool->nr_pages -= chunk->key.len;

	return chunk_destruct(chunk);
}

static int chpool_add(chunk_t *chunk) {
	log_write(LOG_DEBUG, "chpool_add: adding new chunk to chpool...\n");
	chpool_t *cpool = chunk->cpool;

	int err = 0;
	while( cpool->nr_pages + chunk->key.len > cpool->threshold && !(err = chpool_del(cpool->head.next)) );

	if(unlikely(err && err != EBUSY)) {
		return err;
	}

	list_add_tail(&chunk->list, &cpool->head);

	err = map_add(cpool->map, &chunk->key, chunk);
	if (unlikely(err && err != EKEYREJECTED)) {
		log_write(LOG_DEBUG, "map_add: %s", strerror(err));
		return err;
	}

	cpool->nr_pages += chunk->key.len;
	return 0;
}

static int chunk_get(chpool_t *cpool, off_t idx, off_t len, chunk_t **chunk) {
	*chunk = NULL;

	hkey_t key = {{.idx = idx, .len = len}};

	int err = map_lookup_le(cpool->map, &key, chunk);
	if(err) {
		return err;
	}

	if( (*chunk)->key.idx + (*chunk)->key.len < idx + len ) {
		return ENOKEY;
	}

	return 0;
}

static inline off_t get_chunk_idx(chpool_t *cpool, off_t offset) {
	return offset >> cpool->pg_order;
}

static inline off_t get_chunk_len(chpool_t *cpool, off_t offset, size_t size) {
	return ((size + (offset & cpool->pg_mask)) >> cpool->pg_order) + 1;
}

static int chunk_init(off_t idx, off_t len, chpool_t *cpool, chunk_t *chunk) {
	chunk->key.idx = idx;
	chunk->key.len = len;
	chunk->ref_cnt = 0;
	chunk->cpool = cpool;
	chunk->payload = mmap(NULL, chunk->key.len * cpool->pg_sz, cpool->prot, MAP_SHARED, cpool->fd, chunk->key.idx * cpool->pg_sz);
	log_write(LOG_DEBUG, "chunk_init: offset = %jd, chunk->payload == %p\n", chunk->key.idx * cpool->pg_sz, chunk->payload);

	if(chunk->payload == MAP_FAILED) {
		return errno;
	}
	return 0;
}

static int chunk_construct(chpool_t *cpool, off_t idx, off_t len, chunk_t **chunk_ptr) {
	int err;

	if(unlikely((err = mf_malloc(sizeof(chunk_t), (void **)chunk_ptr)))) {
		return err;
	}

	if(unlikely((err = chunk_init(idx, len, cpool, *chunk_ptr)))) {
		return err;
	}

	if(unlikely((err = chpool_add(*chunk_ptr)))) {
		log_write(LOG_DEBUG, "chpool_add: %s\n", strerror(err));
		return err;
	}

	log_write(LOG_DEBUG, "chunk_construct(idx=%jd, len=%jd): success\n", idx, len);
	return 0;
}

int chunk_acquire(chpool_t *cpool,  off_t offset, size_t size, chunk_t **chunk_ptr) {
#ifdef DEBUG
	if(unlikely(!cpool || !chunk_ptr)) {
		return EINVAL;
	}

	BUG_ON(!cpool->map);
#endif

	if(size == 0) {
		*chunk_ptr = NULL;
		return 0;
	}

	off_t idx = get_chunk_idx(cpool, offset);
	off_t len = get_chunk_len(cpool, offset, size);

	int err = chunk_get(cpool, idx, len, chunk_ptr);
	log_write(LOG_DEBUG, "chunk_acquire: chunk_get(idx=%jd, len=%jd): %s\n", idx, len, strerror(err));

	switch(err) {
		case ENOKEY:
			if(unlikely((err = chunk_construct(cpool, idx, len, chunk_ptr)))) {
				return err;
			}
		case 0:
			(*chunk_ptr)->ref_cnt++;
			return 0;
			break;
		default:
			return err;
			break;
	}
}

int chunk_find(chpool_t *cpool, off_t offset, size_t size, chunk_t **chunk) {
#ifdef DEBUG
	if(unlikely(!cpool || !chunk)) {
		return EINVAL;
	}

	BUG_ON(!cpool->map);
#endif

	if(size == 0) {
		*chunk = NULL;
		return 0;
	}

	off_t idx = get_chunk_idx(cpool, offset);
	off_t len = get_chunk_len(cpool, offset, size);

	int err;
	if((err = chunk_get(cpool, idx, len, chunk))) {
		return err;
	}

	log_write(LOG_DEBUG, "chunk_find: success, chunk = %p\n", *chunk);
	(*chunk)->ref_cnt++;
	return 0;
}

int chunk_release(chunk_t *chunk) {
	if(unlikely(chunk == NULL)) {
		return EINVAL;
	}

	if(chunk->ref_cnt == 0) {
		return EAGAIN;
	}

	if(--chunk->ref_cnt == 0) {
		if(chunk->is_indexed) {
			list_move(&chunk->list, &chunk->cpool->head);
		}
		else {
			return chpool_del(&chunk->list);
		}
	}

	return 0;
}

int chpool_construct(int fd, int prot, chpool_t **cpool_ptr) {
#ifdef DEBUG
	if(unlikely(fd < 0 || cpool_ptr == NULL)) {
		return EINVAL;
	}
#endif

	int err;
	if(unlikely((err = mf_malloc(sizeof(chpool_t), (void **)cpool_ptr)))) {
		return err;
	}

	struct sysinfo info = {0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}}; /* Such a initialization is for valgrind & clang */
	sysinfo(&info);

	chpool_t *cpool = *cpool_ptr;

#ifndef DEBUG
	cpool->pg_sz = sysconf(_SC_PAGESIZE) << 14;
#else
	cpool->pg_sz = sysconf(_SC_PAGESIZE);
#endif

	size_t pg_sz = cpool->pg_sz;
	cpool->pg_order = 0;
	cpool->pg_mask = 0;
	while(pg_sz) {
		pg_sz >>= 1;
		cpool->pg_mask <<= 1;
		cpool->pg_mask++;
		cpool->pg_order++;

	}
	if(cpool->pg_order) cpool->pg_order--;

	struct rlimit rl = {0, 0};
	getrlimit(RLIMIT_AS, &rl);

	cpool->threshold = (rl.rlim_cur == RLIM_INFINITY) ? ((info.freeram / cpool->pg_sz) >> 2) * 3 : rl.rlim_cur;
	cpool->nr_pages = 0;
	cpool->fd = fd;
	cpool->prot = prot;
	INIT_LIST_HEAD(&cpool->head);

	struct stat sb = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0, 0}, {0, 0}, {0, 0}, {0}};
	if(unlikely(fstat(fd, &sb) == -1)) {
		return errno;
	}
	cpool->fsize = sb.st_size;

	if(unlikely((err = map_construct(&cpool->map)))) {
		return err;
	}

#ifndef DEBUG2
	chunk_t *chunk;
	size_t ch_sz = (rl.rlim_cur == RLIM_INFINITY) ? (info.freeram >> 1) : min(rl.rlim_cur, (info.freeram >> 1));
	off_t len = get_chunk_len(cpool, 0, min(ch_sz, cpool->fsize));
	err = chunk_construct(cpool, 0, len, &chunk);
#endif

	return 0;
}

int chpool_destruct(chpool_t *cpool) {
#ifdef DEBUG
	if(cpool == NULL) {
		return EINVAL;
	}
#endif

	int err;

	list_t *pos;
	list_t *n;
	list_for_each_safe(pos, n, &cpool->head) {
		chunk_t *tmp = list_entry(pos, chunk_t, list);
		list_del(pos);
		log_write(LOG_DEBUG, "chpool_destruct: destructing chunk %p\n", tmp);
		if(unlikely((err = chunk_destruct(tmp)))) {
			return err;
		}
	}

	map_destruct(cpool->map);

	if(unlikely((err = close(cpool->fd)))) {
		return errno;
	}

	mf_free(cpool);

	return 0;
}

int chunk_get_mem(chunk_t *chunk, off_t offset, void **buf, off_t *border) {
#ifdef DEBUG
	if(unlikely(chunk == NULL || buf == NULL)) {
		return EINVAL;
	}
#endif

	off_t left =  chunk->key.idx * chunk->cpool->pg_sz;
	off_t right = left + chunk->key.len * chunk->cpool->pg_sz;

#ifdef DEBUG
	if(offset < left || offset >= right) {
		return EINVAL;
	}
#endif

	*buf = (char *)chunk->payload + (offset - left);
	if(border) {
		*border = right;
	}

	log_write(LOG_DEBUG, "chunk_get_mem: chunk->payload == %p, *buf = %p\n", chunk->payload, *buf);

	return 0;
}

int chpool_fd(chpool_t *cpool) {
#ifdef DEBUG
	if(unlikely(!cpool)) {
		return -1;
	}
#endif
	return cpool->fd;
}

size_t chpool_page_size(chpool_t *cpool) {
#ifdef DEBUG
	if(unlikely(!cpool)) {
		return 0;
	}
#endif
	return cpool->pg_sz;
}

off_t chpool_fsize(chpool_t *cpool) {
#ifdef DEBUG
	if(unlikely(!cpool)) {
		return -1;
	}
#endif
	return cpool->fsize;	
}