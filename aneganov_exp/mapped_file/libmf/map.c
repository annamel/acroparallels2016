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

/* hash() is a modification of murmur3, adapted for specific case */
#define ROT32(x, y) ((x << y) | (x >> (32 - y))) // avoid effort
uint16_t hash(const hkey_t *key) {
    uint32_t hash = 0;
    uint32_t k;

    for (int i = 0; i < 4; i++) {
        k = key->raw[i]  * 0xcc9e2d51;
        k = ROT32(k, 15) * 0x1b873593;
        hash ^= k;
        hash = ROT32(hash, 13) * 5 + 0xe6546b64;
    }

    hash ^= 16;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

    return hash % HASHTABLE_SIZE;
}

typedef struct elem {
    hkey_t key;
    val_t val;
    bool valid;
} elem_t;

struct map {
    elem_t cache[HASHTABLE_SIZE];
    skiplist_t *skiplist; 
}; /* map_t */

int map_construct(int (*cmp)(val_t, val_t), map_t **map_ptr) {
    int err;
    if(unlikely((err = mf_malloc(sizeof(map_t), (void**)map_ptr)))) {
        return err;
    }

    map_t *map = *map_ptr;
    for(int i = 0; i < HASHTABLE_SIZE; i++ ) {
        map->cache[i].valid = false;
    }

    return skiplist_construct(24, cmp, &map->skiplist);
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
    map->cache[idx].val = val;
    map->cache[idx].valid = true;

    return skiplist_add(map->skiplist, key->idx, val, oldval_ptr);
}

int map_lookup_le(const map_t * map, hkey_t *key, val_t *val_ptr) {
    if(unlikely(map == NULL || key == NULL || val_ptr == NULL))
        return EINVAL;

    unsigned idx = hash(key);
    elem_t elem = map->cache[idx];

    if(elem.valid && elem.key.idx == key->idx && elem.key.len == key->len) {
        *val_ptr = elem.val;
        return 0;
    }

    return skiplist_lookup_le(map->skiplist, key->idx, val_ptr);
}

int map_del(map_t *map, hkey_t *key) {
    if(unlikely(map == NULL || key == NULL))
        return EINVAL;

    unsigned idx = hash(key);
    elem_t elem = map->cache[idx];

    if(elem.key.idx == key->idx && elem.key.len == key->len) {
        elem.valid = false;
    }

    return skiplist_del(map->skiplist, key->idx);
}
