#ifndef __MF_SKIPLIST_H__
#define __MF_SKIPLIST_H__

typedef off_t skey_t;
typedef void* val_t;
typedef struct skiplist skiplist_t;

int skiplist_construct(int max_lvl, int (*inher_cmp)(val_t, val_t), skiplist_t **newlist_ptr);
void skiplist_destruct(skiplist_t *list);
int skiplist_add(skiplist_t *list, skey_t key, val_t val, val_t *oldval_ptr);
int skiplist_del(skiplist_t *list, skey_t key);
int skiplist_get(const skiplist_t *list, skey_t key, val_t *val_ptr);
int skiplist_lookup_le(const skiplist_t *list, skey_t key, val_t *val_ptr);

#endif