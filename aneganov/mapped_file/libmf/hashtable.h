#ifndef __MF_HASHTABLE_H__
#define __MF_HASHTABLE_H__

#include <errno.h>
#include "hash.h"

typedef struct HashTable hashtable_t;

int hashtable_construct(int (*cmp)(hkey_t, hkey_t), size_t (*hash)(hkey_t), hashtable_t **ht);
int hashtable_destruct(hashtable_t **ht);
int hashtable_add(hashtable_t * ht, hkey_t key, hval_t value);
int hashtable_get(const hashtable_t * ht, hkey_t key, hval_t *val);
int hashtable_del(hashtable_t *ht, hkey_t key);

#endif