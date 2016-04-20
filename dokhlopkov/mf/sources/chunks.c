#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "chunks.h"
#include "mallocs.h"
#include "hashtable/hashtable.h"

#define HTBL_SIZE 1024
#define DEFAULT_CPOOL_SIZE 1024

struct Chunk {
	cpool_t *cpool;
	off_t idx;
	off_t len;
	unsigned ref_cnt;
	chunk_t *next;
	chunk_t *prev;
	void *payload;
}; /* typedef chunk_t */

struct ChunkPool {
	int fd;
	size_t size;
	size_t nr_pages;
	chunk_t *ch_list;
	hashtable_t *ht;
	hashtable_t *mem_ht;
}; /* typedef cpool_t */


/* ----- */ /* -PRIVATE- */ /* ----- */
/* ------------ ch's ------------ */
static int ch_init(off_t idx, off_t len, cpool_t *cpool, chunk_t *ch);
static int ch_construct(cpool_t *cpool, off_t idx, off_t len, chunk_t **ch);
static int ch_destruct(chunk_t *ch);
static int ch_get(cpool_t *cpool, off_t idx, chunk_t **ch);
static size_t get_ch_size(off_t multiplier);

/* ------- pool of chs's ------- */
static int cpool_init(cpool_t *cpool, size_t size, int fd);
static int cpool_fini(cpool_t *cpool);
static int cpool_add(chunk_t *ch);
static int cpool_del(chunk_t *ch);


/* ----- */ /* -PUBLIC- */ /* ----- */
/* ------------ ch's ------------ */
int ch_acquire(cpool_t *cpool, off_t offset, size_t size, chunk_t **ch);
int ch_get_mem(chunk_t *ch, off_t offset, void **buf);
int ch_release(chunk_t *ch);
int ch_find(cpool_t *cpool, off_t offset, size_t size, chunk_t **ch);

/* ------- pool of chs's ------- */
int cpool_construct(size_t max_mem, int fd, cpool_t **cpool);
int cpool_destruct(cpool_t **cpool);
int cpool_mem_add(void *ptr, chunk_t *ch);
int cpool_mem_get(cpool_t *cpool, void *ptr, chunk_t **ch);
int cpool_fd(cpool_t *cpool);


/* ----- */ /* -CODE- */ /* ----- */
static size_t get_ch_size(off_t count) {
	return (size_t)count * sysconf(_SC_PAGESIZE);
}

static int ch_init(off_t idx, off_t len, cpool_t *cpool, chunk_t *ch) {
	ch->idx = idx;
	ch->len = len;
	ch->ref_cnt = 1;
	ch->prev = NULL;
	ch->next = NULL;
	ch->cpool = cpool;
	ch->payload = mmap(NULL,
					   get_ch_size(ch->len),
					   PROT_READ | PROT_WRITE, MAP_SHARED,
					   cpool->fd,
					   (off_t)get_ch_size(idx));

    if (ch->payload == MAP_FAILED) return errno;
	return 0;
}

static int ch_destruct(chunk_t *ch) {
	if (munmap(ch->payload, ch->len * get_ch_size(1)) == -1) return errno;
    return _free((void **)&ch);
}

static int cpool_del(chunk_t *ch) {
	if (ch == NULL || ch->ref_cnt != 0) return EBUSY;

	cpool_t *cpool = ch->cpool;

	if(ch->prev) ch->prev->next = ch->next;
	if(ch->next) ch->next->prev = ch->prev;

	int err;
	for(unsigned i = 0; i < ch->len; ++i) {
		err = hashtable_delete(cpool->ht, ch->idx + i);
		if (err && err != ENODATA) return err;
	}

	cpool->nr_pages -= ch->len;

	err = ch_destruct(ch);
	if(err) return err;

	return 0;
}

static int cpool_add(chunk_t *ch) {
	cpool_t *cpool = ch->cpool;

	int err = 0;
	while (cpool->nr_pages + ch->len > cpool->size && !(err = cpool_del(cpool->ch_list)));

	if(err == EBUSY) return ENOBUFS;
	if(err) return err;

	ch->next = cpool->ch_list;
	ch->prev = NULL;
	cpool->ch_list = ch;

	for (unsigned i = 0; i < ch->len; ++i) {
		err = hashtable_add(cpool->ht, ch->idx+i, (hval_t)ch);
		if(err) return err;
	}

	cpool->nr_pages += ch->len;
	return 0;
}

static int ch_construct(cpool_t *cpool, off_t idx, off_t len, chunk_t **ch) {
	int err = _malloc(sizeof(chunk_t), (void **)ch);
	if(err) return err;

	err = ch_init(idx, len, cpool, *ch);
	if(err) return err;

	err = cpool_add(*ch);
	if(err) return err;

	return 0;
}

static int ch_get(cpool_t *cpool, off_t idx, chunk_t **ch) {
	*ch = NULL;

	*ch = hashtable_get(cpool->ht, idx);
	if (*ch == NULL) return ENODATA;

	return 0;
}

int ch_acquire(cpool_t *cpool, off_t offset, size_t size, chunk_t **ch) {
	if (!cpool || !ch || !cpool->ht || cpool->fd < 0 ||
		  cpool->nr_pages > cpool->size) return EINVAL;

	if (size == 0) {
		*ch = NULL;
		return 0;
	}

	off_t idx = (off_t) (offset / get_ch_size(1));
	off_t len = (off_t) (size / get_ch_size(1) + 1);

	int err = ch_get(cpool, idx, ch);

	if (err == ENODATA) {
		if( *ch != NULL ) {
			err = cpool_del(*ch);
			if (err && err != EBUSY) return err;
		}
		ch_construct(cpool, idx, len, ch);
		return 0;
	}
	else return err;
}

int ch_find(cpool_t *cpool, off_t offset, size_t size, chunk_t **ch) {
	if (!cpool || !ch || !cpool->ht || cpool->fd < 0 ||
		   cpool->nr_pages > cpool->size) return EINVAL;

	if (size == 0) {
		*ch = NULL;
		return 0;
	}

	off_t idx = (off_t) (offset / get_ch_size(1));

	return ch_get(cpool, idx, ch);
}

int ch_release(chunk_t *ch) {
	if (!ch) return EINVAL;

	if (ch->ref_cnt == 0) return EAGAIN;

	if (--ch->ref_cnt == 0) {
		if(ch->prev) ch->prev->next = ch->next;
		if(ch->next) ch->next->prev = ch->prev;
		ch->next = ch->cpool->ch_list;
		ch->prev = NULL;
	}

	return 0;
}

static int cpool_init(cpool_t *cpool, size_t size, int fd) {
	cpool->ch_list = NULL;
	cpool->size = size;
	cpool->nr_pages = 0;
	cpool->fd = fd;
  	cpool->ht = hashtable_construct(HTBL_SIZE);
  	cpool->mem_ht = hashtable_construct(HTBL_SIZE);
	return 0;
}

int cpool_construct(size_t max_mem, int fd, cpool_t **cpool) {
	size_t size = max_mem ? max_mem / get_ch_size(1) : DEFAULT_CPOOL_SIZE;

	int err = _malloc (sizeof(cpool_t), (void **)cpool);
	if (err) return err;

	return cpool_init(*cpool, size, fd);
}

static int cpool_fini(cpool_t *cpool) {
	while(cpool->ch_list) {
		chunk_t *iter = cpool->ch_list;
		cpool->ch_list = iter->next;

		int err = ch_destruct(iter);
		if(err) return err;
	}

	cpool->nr_pages = 0;

	int err = hashtable_destruct(cpool->ht);
	if(err) return err;

	err = hashtable_destruct(cpool->mem_ht);
	if(err) return err;

	err = close(cpool->fd);
	if(err == -1) return errno;

	return 0;
}

int cpool_destruct(cpool_t **cpool) {
	if (cpool == NULL)
		return EINVAL;

	int err = cpool_fini(*cpool);
	if (err) return err;

	err = _free((void **)cpool);
	if (err) return err;

	return 0;
}

int ch_get_mem(chunk_t *ch, off_t offset, void **buf) {
	if (ch == NULL || buf == NULL) return EINVAL;

	off_t left = (off_t) (get_ch_size(ch->idx));
	off_t right = (off_t) (left + get_ch_size(ch->len));

	if (offset < left || offset > right)
		return EINVAL;

	*buf = ch->payload + offset - left;
	return 0;
}

int cpool_fd(cpool_t *cpool) {
	if (!cpool || !cpool->ht || cpool->fd < 0 ||
		  cpool->nr_pages > cpool->size) return -1;
	return cpool->fd;
}
