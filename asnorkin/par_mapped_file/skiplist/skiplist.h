#ifndef SKIPLIST_H
#define SKIPLIST_H

#include "../typedefs.h"
#include "sys/mman.h"


struct snode
{
    slkey_t key;
    slvalue_t value;
    struct snode **forward;
};



struct skiplist
{
    int level;
    int size;
    struct snode *header;
};


skiplist_t *skiplist_init();
int skiplist_add(skiplist_t *list, slkey_t key, slvalue_t value);
int skiplist_find(skiplist_t *list, off_t index, off_t len, chunk_t **chunk);
int skiplist_del(skiplist_t *list, slkey_t key);
int skiplist_deinit(skiplist_t *list);



#endif // SKIPLIST_H
