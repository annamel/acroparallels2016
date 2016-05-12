#define _GNU_SOURCE 1
#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "chunks.h"
#include "mallocs.h"
#include "hashtable/hashtable.h"

#define HTBL_SIZE 2048
#define DEFAULT_CPOOL_SIZE 20480

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
	size_t threshold;
	size_t nr_pages;
	chunk_t *head;
	hashtable_t *ht;
}; /* typedef cpool_t */


/* ----- */ /* -PRIVATE- */ /* ----- */
/* ------------ ch's ------------ */
static int ch_init(off_t idx, off_t len, cpool_t *cpool, chunk_t *ch);
static int ch_construct(cpool_t *cpool, off_t idx, off_t len, chunk_t **ch);
static int ch_destruct(chunk_t *ch);
static int ch_get(cpool_t *cpool, off_t idx, off_t len, chunk_t **ch);
static size_t get_ch_size(off_t multiplier);

/* ------- pool of chs's ------- */
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
int cpool_destruct(cpool_t *cpool);
int cpool_mem_add(void *ptr, chunk_t *ch);
int cpool_mem_get(cpool_t *cpool, void *ptr, chunk_t **ch);
int cpool_fd(cpool_t *cpool);


/* ----- */ /* -CODE- */ /* ----- */
static size_t get_ch_size(off_t count) {
	return (size_t)count * sysconf(_SC_PAGESIZE) << 4;
}

static int ch_init(off_t idx, off_t len, cpool_t *cpool, chunk_t *ch) {
	ch->idx = idx;
	ch->len = len;
	ch->ref_cnt = 1;
	ch->next = ch;
	ch->prev = ch;
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
	if (munmap(ch->payload, get_ch_size(ch->len)) == -1) return errno;
    return _free(sizeof(chunk_t), (void *)ch);
}

static int cpool_del(chunk_t *ch) {
	if (ch == NULL || ch->ref_cnt != 0) return EBUSY;

	if (ch != ch->next) {
		ch->prev->next = ch->next;
		ch->next->prev = ch->prev;
		if (ch == ch->cpool->head) ch->cpool->head = ch->next;
	} else ch->cpool->head = NULL;

	int err;
	for(unsigned i = 0; i < ch->len; ++i) {
		err = hashtable_delete(ch->cpool->ht, ch->idx + i);
		if (err && err != ENODATA) return err;
	}

	ch->cpool->nr_pages -= ch->len;

	err = ch_destruct(ch);
	if (err) return err;

	return 0;
}

static int cpool_add(chunk_t *ch) {
	int err = 0;
	while (ch->cpool->nr_pages + ch->len > ch->cpool->threshold && !(err = cpool_del(ch->cpool->head)));

	if (err == EBUSY) return ENOBUFS;
	if (err) return err;

	if (ch->cpool->head != NULL) {
		ch->next = ch->cpool->head;
		ch->prev = ch->cpool->head->prev;
		ch->next->prev = ch;
		ch->prev->next = ch;
	} else ch->cpool->head = ch;

	for (unsigned i = 0; i < ch->len; ++i) {
		err = hashtable_add(ch->cpool->ht, ch->idx+i, (hval_t)ch);
		if (err) return err;
	}

	ch->cpool->nr_pages += ch->len;
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

static int ch_get(cpool_t *cpool, off_t idx, off_t len, chunk_t **ch) {
	*ch = NULL;

	*ch = hashtable_get(cpool->ht, idx);
	if (*ch == NULL) return ENODATA;
	if ((*ch)->idx + (*ch)->len < idx + len) return ENODATA;

	return 0;
}

int ch_acquire(cpool_t *cpool, off_t offset, size_t size, chunk_t **ch_ptr) {
	if (!cpool || !ch_ptr || !cpool->ht || cpool->fd < 0) return EINVAL;

	if (size == 0) {
		*ch_ptr = NULL;
		return 0;
	}

	off_t idx = (off_t) (offset / get_ch_size(1));
	off_t len = (off_t) (size / get_ch_size(1) + 1);

	int err = ch_get(cpool, idx, len, ch_ptr);

	chunk_t *ch = *ch_ptr;

	switch(err) {
		case 0:
			ch->ref_cnt++;
			return 0;
		case ENODATA:
			if (ch == NULL || !(err = cpool_del(ch)))
				return ch_construct(cpool, idx, len, ch_ptr);
			if (err != EBUSY) return err;
			void *newpl = mremap(ch->payload, get_ch_size(ch->len), get_ch_size(idx + len - ch->idx), 0);
			if (newpl != MAP_FAILED) {
				ch->payload = newpl;
				ch->ref_cnt++;
				return 0;
			} else {
				if (errno != ENOMEM) return errno;
				return ch_construct(cpool, idx, len, ch_ptr);
			}
		default:
			return err;
	}
}

int ch_find(cpool_t *cpool, off_t offset, size_t size, chunk_t **ch) {
	if (!cpool || !ch || !cpool->ht || cpool->fd < 0 ||
		   cpool->nr_pages > cpool->threshold) return EINVAL;

	if (size == 0) {
		*ch = NULL;
		return 0;
	}

	off_t idx = (off_t) (offset / get_ch_size(1));
	off_t len = (off_t) (size / get_ch_size(1) + 1);

	int err = ch_get(cpool, idx, len, ch);
	if (err) return err;
	(*ch)->ref_cnt++;
	return 0;

}

int ch_release(chunk_t *ch) {
	if (!ch) return EINVAL;

	if (ch->ref_cnt == 0) return EAGAIN;

	if (--ch->ref_cnt == 0 && ch != ch->cpool->head) {
		ch->prev->next = ch->next;
		ch->next->prev = ch->prev;

		chunk_t *old = ch->cpool->head;

		ch->next = old;
		ch->prev->next = ch;

		ch->next->prev = ch;
		ch->prev->next = ch;

		ch->cpool->head = ch;
	}

	return 0;
}

int cpool_construct(size_t max_mem, int fd, cpool_t **cpool_ptr) {
	if (fd < 0 || cpool_ptr == NULL) return EINVAL;

	size_t size = max_mem ? max_mem / get_ch_size(1) : DEFAULT_CPOOL_SIZE;

	int err = _malloc (sizeof(cpool_t), (void **)cpool_ptr);
	if (err) return err;

	cpool_t *cpool = *cpool_ptr;

	cpool->head = NULL;
	cpool->threshold = size;
	cpool->nr_pages = 0;
	cpool->fd = fd;
	cpool->ht = hashtable_construct(HTBL_SIZE);

	if (cpool->ht == NULL) return 1;
	return err;
}

int cpool_destruct(cpool_t *cpool) {
	if (cpool == NULL) return EINVAL;

	if (cpool->head) {
		cpool->head->prev->next = NULL;
		cpool->head->prev = NULL;
	}

	while (cpool->head) {
		chunk_t *iter = cpool->head;
		cpool->head = iter->next;

		int err = ch_destruct(iter);
		if(err) return err;
	}

	cpool->nr_pages = 0;

	int err = hashtable_destruct(cpool->ht);
	if (err) return err;

	err = close(cpool->fd);
	if (err == -1) return errno;

	err = _free (sizeof(cpool_t), (void *)cpool);
	if (err) return err;

	return 0;
}

int ch_get_mem(chunk_t *ch, off_t offset, void **buf) {
	if (ch == NULL || buf == NULL) return EINVAL;

	off_t left = (off_t) (get_ch_size(ch->idx));
	off_t right = (off_t) (left + get_ch_size(ch->len));

	if (offset < left || offset > right) return EINVAL;

	*buf = ch->payload + offset - left;
	return 0;
}

int cpool_fd(cpool_t *cpool) {
	if (!cpool || cpool->fd < 0) return -1;
	return cpool->fd;
}
