/* 
Skip Lists are a probabilistic alternative to balanced trees, as
described in the June 1990 issue of CACM and were invented by 
William Pugh in 1987. 

This implementation is based on the code of Gray Watson implementation (http://256.com/sources/skip/docs/),
but there is a number of significant differences.

The insertion routine has been implemented so as to use the
dirty hack described in the CACM paper: if a random level is
generated that is more than the current maximum level, the
current maximum level plus one is used instead.
*/

#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdint.h>

#include "mf_malloc.h"
#include "log.h"
#include "mfdef.h"
#include "skiplist.h"

typedef struct skiplist_node {
	skey_t key;
	val_t val;
	struct skiplist_node * forward[0];
} node_t;

typedef struct skiplist { 
	node_t * head;
	node_t nil;
	int lvl;
	int max_lvl;
	int (*inher_cmp)(val_t, val_t);
} skiplist_t;

int skiplist_construct(int max_lvl, int (*inher_cmp)(val_t, val_t), skiplist_t **newlist_ptr) {
	if(unlikely(newlist_ptr == NULL || max_lvl < 0)) {
		return EINVAL;
	}

	int err;
	if(unlikely((err = mf_malloc(sizeof(skiplist_t), (void **)newlist_ptr)))) {
		return err;
	}

	skiplist_t *newlist = *newlist_ptr;
	newlist->lvl = 0;
	newlist->max_lvl = max_lvl;
	newlist->inher_cmp = inher_cmp;
	newlist->nil.key = (skey_t)((uint64_t)(-1) >> 1);
	log_write(LOG_DEBUG, "nil.key = %jx\n", newlist->nil.key);

	if(unlikely((err = mf_malloc(sizeof(node_t) + (max_lvl+2) * sizeof(node_t *), (void **)&newlist->head)))) {
		return err;
	}

	for(int i = 0; i < max_lvl + 2; i++)
		newlist->head->forward[i] = &newlist->nil;

	return 0;
}

void skiplist_destruct(skiplist_t *list) {
	if(list == NULL) {
		return;
	}

	node_t * p = list->head;
	node_t *q;
	do {
		q = p->forward[0];
		mf_free(p);
		p = q;
	} while (p != &list->nil);
	mf_free(list);
}

static int random_level(skiplist_t *list) {
    int level = 0;
    while (rand() < RAND_MAX/2)
        level++;
    return level > list->max_lvl ? list->max_lvl : level;
}

int skiplist_add(skiplist_t *list, skey_t key, val_t val, val_t *oldval_ptr) {
	if(unlikely(list == NULL || oldval_ptr == NULL)) {
		return EINVAL;
	}

	log_write(LOG_DEBUG, "skiplist_add: val = %p\n", val);

	int err = 0;
	*oldval_ptr = NULL;

	node_t **update;
	if (unlikely((err = mf_malloc( (list->max_lvl + 1) * sizeof(node_t *), (void **)&update)))) {
		return err;
	}

	node_t *p = list->head;
	node_t *q;
	int k = list->lvl;

	do {
		while (q = p->forward[k], q->key < key) {
			p = q;
		}
		update[k] = p;
	} while(--k >= 0);

	if (q->key == key) {
		if(list->inher_cmp(val, q->val) == 1) {
			*oldval_ptr = q->val;
			q->val = val;
			return 0;
		}
		else return EKEYREJECTED;
	};

	k = random_level(list);
	if (k > list->lvl) {	
		k = ++list->lvl;
		update[k] = list->head;
	};

	if (unlikely((err = mf_malloc(sizeof(node_t) + (k+1) * sizeof(node_t *), (void **)&q)))) {
		goto done;
	}

	q->key = key;
	q->val = val;

	do {
		p = update[k];
		q->forward[k] = p->forward[k];
		p->forward[k] = q;
	} while(--k >= 0);

done:
	mf_free(update);
	return err;
}

int skiplist_del(skiplist_t *list, skey_t key) {
	if(unlikely(list == NULL)) {
		return EINVAL;
	}

	int err = 0;
	node_t **update = NULL;
	if (unlikely((err = mf_malloc( (list->max_lvl + 1) * sizeof(node_t *), (void **)&update)))) {
		return err;
	}

	node_t *p = list->head;
	node_t *q;
	int k = list->lvl;
	int m = list->lvl;

	do {
		while (q = p->forward[k], q->key < key) {
			p = q;
		}
		update[k] = p;
	} while(--k >= 0);

	if (q->key == key) {
		for(k = 0; k <= m && (p = update[k])->forward[k] == q; k++) {
			p->forward[k] = q->forward[k];
		}
		mf_free(q);

		while( list->head->forward[m] == &list->nil && m > 0 ) {
			m--;
		}

		list->lvl = m;
	}
	else {
		err = ENOKEY;
	}

	mf_free(update);
	return err;
}

int skiplist_get(const skiplist_t *list, skey_t key, val_t *val_ptr) {
	if(unlikely(list == NULL || val_ptr == NULL)) {
		return EINVAL;
	}

	int k = list->lvl;
	node_t *p = list->head;
	node_t *q;

	do {
		while (q = p->forward[k], q->key < key) {
			p = q;
		}
	} while (--k >= 0);

	if (q->key != key)
		return ENOKEY;

	*val_ptr = q->val;

	return 0;
}

int skiplist_lookup_le(const skiplist_t *list, skey_t key, val_t *val_ptr) {
	if(unlikely(list == NULL || val_ptr == NULL)) {
		return EINVAL;
	}

	int k = list->lvl;
	node_t *p = list->head;
	node_t *q;

	do {
		while (q = p->forward[k], q->key < key) {
			p = q;
		}
	} while (--k >= 0);

	if (q->key != key) {
		if(p->key >= key || p->val == NULL) {
			return ENOKEY;
		}
		else q = p;
	}

	log_write(LOG_DEBUG, "skiplist_lookup_le: q->val = %p\n", q->val);
	*val_ptr = q->val;

	return 0;
}