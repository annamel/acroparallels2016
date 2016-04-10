#include <string.h>

#include "mf_malloc.h"
#include "hashtable.h"
#include "log.h"

typedef struct Elem {
    hkey_t key;
    hval_t val;
} elem_t;

typedef struct HashTableRow {
    size_t len;
    size_t data_len;
    elem_t *data;
} htablerow_t;

struct HashTable {
    size_t (*hash_fn)(hkey_t);
    int (*cmp_fn)(hkey_t, hkey_t);
    htablerow_t payload[HASHTABLE_SIZE];
}; /* hashtable_t */

#define MIN_ROW_LEN   (64)

static int hashtable_init(hashtable_t *ht, int (*cmp)(hkey_t, hkey_t), size_t (*hash)(hkey_t)) {
    if( ht == NULL || cmp == NULL || hash == NULL )
        return EINVAL;

	ht->cmp_fn  = cmp;
    ht->hash_fn = hash;

    int i = 0;
    for( i = 0; i < HASHTABLE_SIZE; i++ ) {
        ht->payload[i].data_len = 0;
        ht->payload[i].len = MIN_ROW_LEN;
        ht->payload[i].data = NULL;
    }

    int res = 0;
    for(i = 0; i < HASHTABLE_SIZE; i++)
        if( (res = mf_malloc(MIN_ROW_LEN*sizeof(elem_t), (void **)&ht->payload[i].data)) )
            return res;

    return 0;
}

static int hashtable_fini(hashtable_t *ht) {
    if(ht == NULL)
        return EINVAL;

    ht->cmp_fn = NULL;
    ht->hash_fn = NULL;

    int i = 0;
    for( i = 0; i < HASHTABLE_SIZE; i++ ) {
        mf_free(ht->payload[i].len, (void **)&ht->payload[i].data);
        ht->payload[i].data_len = 0;
        ht->payload[i].len = 0;
    }

    return 0;
}

int hashtable_construct(int (*cmp)(hkey_t, hkey_t), size_t (*hash)(hkey_t), hashtable_t **ht) {
    int err = mf_malloc( sizeof(hashtable_t), (void**)ht );
    if(err) return err;
    return hashtable_init(*ht, cmp, hash);
}

int hashtable_destruct(hashtable_t **ht) {
    int err = hashtable_fini(*ht);
    if(err) return err;
    return mf_free( sizeof(hashtable_t), (void**)ht );
}

int hashtable_add(hashtable_t * ht, hkey_t key, hval_t val) {
    if(ht == NULL)
        return EINVAL;

    unsigned idx = ht->hash_fn(key) % HASHTABLE_SIZE;
    size_t data_len = ht->payload[idx].data_len;
    size_t len = ht->payload[idx].len;
    elem_t *data = ht->payload[idx].data;

    if( data_len > len )
        return EINVAL;

    int pos = 0;
    while( pos < data_len && ht->cmp_fn(key, data[pos].key) == -1 )
        pos++;

    if( pos < data_len && ht->cmp_fn(key, data[pos].key) == 0 ) {
        data[pos].val = val;
        return 0;
    }

    if( data_len == len ) {
        int res = mf_realloc(2*len, (void **)&ht->payload[idx].data);
        if( res )
            return res;
        len = ht->payload[idx].len *= 2;
        data = ht->payload[idx].data;
    }
    data_len = ++ht->payload[idx].data_len;

    if(pos < data_len - 1)
        memmove(data + pos + 1, data + pos, data_len - pos - 1);

    data[pos].key = key;
    data[pos].val = val;

    return 0;
}

int hashtable_get(const hashtable_t * ht, hkey_t key, hval_t *val) {
    if(ht == NULL)
        return EINVAL;

    unsigned idx = ht->hash_fn(key) % HASHTABLE_SIZE;

    size_t data_len = ht->payload[idx].data_len;
    size_t len = ht->payload[idx].len;
    elem_t *data = ht->payload[idx].data;

    if( data_len > len )
        return EINVAL;

    if(data_len == 0)
        return ENOKEY;

    int mid = data_len/2;
    int left = 0;
    int right = data_len - 1;

    while( ht->cmp_fn(key, data[mid].key) != 0 ) {
        if(right - left <= 1)
            break;
        if( ht->cmp_fn(key, data[mid].key) == -1 )
            left = mid;
        if( ht->cmp_fn(key, data[mid].key) == 1 )
            right = mid;
        mid = left + (right - left + 1)/2;
    }
    if( ht->cmp_fn(key, data[mid].key) != 0 )
        return ENOKEY;

    *val = data[mid].val;
    return 0;
}

int hashtable_del(hashtable_t *ht, hkey_t key) {
    if(ht == NULL)
        return EINVAL;

    unsigned idx = ht->hash_fn(key) % HASHTABLE_SIZE;
    size_t data_len = ht->payload[idx].data_len;
    size_t len = ht->payload[idx].len;
    elem_t *data = ht->payload[idx].data;

    if( data_len > len )
        return EINVAL;

    int pos = 0;
    while( pos < data_len && ht->cmp_fn(key, data[pos].key) == -1 )
        pos++;

    if(pos == data_len || ht->cmp_fn(key, data[pos].key) == 1)
        return ENOKEY;

    if(pos < data_len)
        memmove(data + pos, data + pos + 1, data_len - pos - 1);

    ht->payload[idx].data_len--;

    return 0;
}
