#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "mf_malloc.h"
#include "mfdef.h"
#include "log.h"
#include "bug.h"
#include "skiplist.h"
#include "map.h"

#define HASHTABLE_SIZE 1024

static inline uint16_t hash(const hkey_t *key) {
    return key->idx % HASHTABLE_SIZE;
}

typedef struct elem {
    hkey_t key;
    val_t val;
    bool valid;
} elem_t;

struct map {
    int (*cmp)(val_t, val_t);
    off_t (*skey_from_val)(val_t);
    elem_t cache[HASHTABLE_SIZE];
    skiplist_t *skiplist; 
}; /* map_t */

int map_construct(int (*cmp)(val_t, val_t), off_t (*skey_from_val)(val_t), map_t **map_ptr) {
    int err;
    if(unlikely((err = mf_malloc(sizeof(map_t), (void**)map_ptr)))) {
        return err;
    }

    map_t *map = *map_ptr;
    for(int i = 0; i < HASHTABLE_SIZE; i++ ) {
        map->cache[i].valid = false;
    }

    map->cmp = cmp;
    map->skey_from_val = skey_from_val;

    return skiplist_construct(24, &map->skiplist);
}

int map_destruct(map_t *map) {
    skiplist_destruct(map->skiplist);
    return 0;
}

int map_add(map_t * map, hkey_t *key, val_t val, val_t *oldval_ptr) {
    if(unlikely(map == NULL || key == NULL)) {
        return EINVAL;
    }

    unsigned idx = hash(key);
    map->cache[idx].key.idx = key->idx;
    map->cache[idx].key.len = key->len;

    int err = skiplist_lookup_le(map->skiplist, key->idx, oldval_ptr);
    if(unlikely(err && err != ENOKEY)) {
        return err;
    }

    if(!err && map->cmp(*oldval_ptr, val) == 1) {
        map->cache[idx].val = *oldval_ptr;
        map->cache[idx].valid = true;
        return EKEYREJECTED;
    }

    map->cache[idx].val = val;
    map->cache[idx].valid = true;

    err = skiplist_lookup_ge(map->skiplist, key->idx, oldval_ptr);
    if(unlikely(err && err != ENOKEY)) {
        return err;
    }

    if(!err && map->cmp(*oldval_ptr, val) == -1) {
        if(unlikely((err = skiplist_del(map->skiplist, map->skey_from_val(*oldval_ptr))))) {
            *oldval_ptr = NULL;
            return err;
        }
    }
    else *oldval_ptr = NULL;

    return skiplist_add(map->skiplist, key->idx, val);
}

int map_lookup_le(const map_t * map, hkey_t *key, val_t *val_ptr) {
    if(unlikely(map == NULL || key == NULL || val_ptr == NULL))
        return EINVAL;

    elem_t elem = map->cache[hash(key)];

    if(elem.valid && elem.key.idx == key->idx && elem.key.len >= key->len) {
        *val_ptr = elem.val;
        return 0;
    }

    return skiplist_lookup_le(map->skiplist, key->idx, val_ptr);
}

int map_del(map_t *map, hkey_t *key, bool is_indexed) {
    if(unlikely(map == NULL || key == NULL))
        return EINVAL;

    map->cache[hash(key)].valid = false;

    return is_indexed ? skiplist_del(map->skiplist, key->idx) : 0;
}