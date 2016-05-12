#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "mf_malloc.h"
#include "mfdef.h"
#include "log.h"
#include "bug.h"
#include "skiplist.h"
#include "chunk.h"
#include "map.h"

#define HASHTABLE_SIZE 1024

static inline uint16_t hash(const hkey_t *key) {
    return key->idx % HASHTABLE_SIZE;
}

typedef struct elem {
    hkey_t key;
    hval_t val;
    bool valid;
} elem_t;

struct map {
    elem_t cache[HASHTABLE_SIZE];
    skiplist_t *skiplist; 
}; /* map_t */

int map_construct(map_t **map_ptr) {
    int err;
    if(unlikely((err = mf_malloc(sizeof(map_t), (void**)map_ptr)))) {
        return err;
    }

    map_t *map = *map_ptr;
    for(int i = 0; i < HASHTABLE_SIZE; i++ ) {
        map->cache[i].valid = false;
    }

    return skiplist_construct(24, &map->skiplist);
}

int map_destruct(map_t *map) {
#ifdef DEBUG
    if(unlikely(map == NULL)) {
        return EINVAL;
    }
    for(int i = 0; i < HASHTABLE_SIZE; i++ ) {
        map->cache[i].valid = false;
    }  
#endif

    skiplist_destruct(map->skiplist);
    return 0;
}

static int right_cmp(hval_t val1, hval_t val2) {
    off_t right1 = val1->key.idx + val1->key.len;
    off_t right2 = val2->key.idx + val2->key.len;
    return (right1 > right2) ? 1 : (right1 < right2) ? -1 : 0;
}

int map_add(map_t * map, hkey_t *key, hval_t val) {
#ifdef DEBUG
    if(unlikely(map == NULL || key == NULL)) {
        return EINVAL;
    }
#endif

    unsigned idx = hash(key);
    map->cache[idx].key.idx = key->idx;
    map->cache[idx].key.len = key->len;

    hval_t oldval;

    int err = skiplist_lookup_le(map->skiplist, key->idx, (val_t *)&oldval);
    if(unlikely(err && err != ENOKEY)) {
        return err;
    }

    if(!err && right_cmp(oldval, val) > -1) {
        map->cache[idx].val = oldval;
        map->cache[idx].valid = true;
        val->is_indexed = false;
        return EKEYREJECTED;
    }

    while(true) {
        err = skiplist_lookup_ge(map->skiplist, key->idx, (val_t *)&oldval);
        if(unlikely(err && err != ENOKEY)) {
            return err;
        }

        if(err == ENOKEY) {
            break;
        }

        if( right_cmp(oldval, val) < 1 ) {
            if(unlikely((err = skiplist_del(map->skiplist, oldval->key.idx)))) {
                return err;
            }
            oldval->is_indexed = false;
            map->cache[hash(&oldval->key)].valid = false;
        }
        else {
            break;
        }
    }

    val->is_indexed = true;
    map->cache[idx].val = val;
    map->cache[idx].valid = true;

    return skiplist_add(map->skiplist, key->idx, val);
}

int map_lookup_le(const map_t * map, hkey_t *key, hval_t *val_ptr) {
#ifdef DEBUG
    if(unlikely(map == NULL || key == NULL || val_ptr == NULL))
        return EINVAL;
#endif

    elem_t elem = map->cache[hash(key)];

    if(elem.valid && elem.key.idx == key->idx && elem.key.len >= key->len) {
        *val_ptr = elem.val;
        return 0;
    }

    return skiplist_lookup_le(map->skiplist, key->idx, (val_t *)val_ptr);
}

int map_del(map_t *map, hkey_t *key, bool is_indexed) {
#ifdef DEBUG
    if(unlikely(map == NULL || key == NULL))
        return EINVAL;
#endif

    map->cache[hash(key)].valid = false;

    return is_indexed ? skiplist_del(map->skiplist, key->idx) : 0;
}
