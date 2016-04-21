#include <string.h>

#include "mf_malloc.h"
#include "hashtable.h"
#include "log.h"
#include "bug.h"

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

#define FOUND    1
#define NOTFOUND 0
#define ERROR   -1

static int bin_search(elem_t *data, size_t data_len, hkey_t key, int (*cmp_fn)(hkey_t, hkey_t), size_t *pos)
{
    if(data == NULL || cmp_fn == NULL || pos == NULL)
        return ERROR;

    size_t left = 0;
    size_t right = data_len; 

    if (data_len == 0) {
        *pos = 0;
        return NOTFOUND;
    } else if ( cmp_fn(data[0].key, key) == 1 ) {
        *pos = 0;
        return NOTFOUND;
    } else if ( cmp_fn(data[data_len - 1].key, key) == -1) {
        *pos = data_len;
        return NOTFOUND;
    }

    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if ( cmp_fn(data[mid].key, key) > -1 )
            right = mid;
        else
            left = mid + 1;
    }

    if ( cmp_fn(data[right].key, key) == 0 ) {
        *pos = right;
        return FOUND;
    } else {
        *pos = right;
        return NOTFOUND;
    }
}

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

    log_write(LOG_INFO, "hashtable initiated: cmp_fn = %p, hash_fn = %p\n", cmp, hash);

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

    log_write(LOG_INFO, "hashtable finalized\n");

    return 0;
}

int hashtable_construct(int (*cmp)(hkey_t, hkey_t), size_t (*hash)(hkey_t), hashtable_t **ht) {
    int err = mf_malloc( sizeof(hashtable_t), (void**)ht );
    if(err) return err;
    return hashtable_init(*ht, cmp, hash);
}

int hashtable_destruct(hashtable_t **ht) {
    if(ht == NULL) return EINVAL;
    int err = hashtable_fini(*ht);
    if(err) return err;
    return mf_free( sizeof(hashtable_t), (void**)ht );
}

int hashtable_add(hashtable_t * ht, hkey_t key, hval_t val) {
    if(ht == NULL) return EINVAL;

    unsigned idx = ht->hash_fn(key) % HASHTABLE_SIZE;
    size_t data_len = ht->payload[idx].data_len;
    size_t len = ht->payload[idx].len;
    elem_t *data = ht->payload[idx].data;

    BUG_ON( data_len > len );

    size_t pos = 0;
    int found = bin_search(data, data_len, key, ht->cmp_fn, &pos);
    BUG_ON(found == ERROR);
    
    if(found == FOUND)
        goto end;

    if( data_len == len ) {
        len = ht->payload[idx].len *= 2;
        int err = mf_realloc(len*sizeof(elem_t), (void **)&ht->payload[idx].data);
        if(err) return err;
        data = ht->payload[idx].data;
    }
    data_len = ++ht->payload[idx].data_len;

    if(pos < data_len - 1)
        memmove( (void*)data + (pos+1)*sizeof(elem_t), (void*)data + pos*sizeof(elem_t), (data_len-pos-1)*sizeof(elem_t) );

    data[pos].key = key;

end:
    data[pos].val = val;

    log_write(LOG_INFO, "hashtable_add: idx=%u, pos=%zd, key=%jd, val=%p\n", idx, pos, key, val);
    log_write(LOG_DEBUG, "hashtable_add: data_len = %zu\n", data_len);

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

    size_t pos = 0;
    int found = bin_search(data, data_len, key, ht->cmp_fn, &pos);
    BUG_ON(found == ERROR);

    if(found == NOTFOUND) {
        log_write(LOG_INFO, "hashtable_get: key=%jd: no such key\n", key);
        for(int i=0; i<data_len; i++)
            log_write(LOG_DEBUG, "hashtable_get: data[%d].key = %jd\n", i, data[i].key);
        return ENOKEY;
    }

    *val = data[pos].val;

    log_write(LOG_INFO, "hashtable_get: idx=%u, pos=%zd, key=%jd, val=%p\n", idx, pos, key, *val);
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

    size_t pos = 0;
    int found = bin_search(data, data_len, key, ht->cmp_fn, &pos);
    BUG_ON(found == ERROR);

    if(found == NOTFOUND) {
        log_write(LOG_INFO, "hashtable_del: key=%jd: no such key\n", key);
        return ENOKEY;
    }

    memmove( (void*)data + pos*sizeof(elem_t), (void*)data + (pos+1)*sizeof(elem_t), (data_len-pos-1)*sizeof(elem_t) );

    ht->payload[idx].data_len--;

    log_write(LOG_INFO, "hashtable_del: idx=%u, pos=%zd, key=%jd\n", idx, pos, key);
    return 0;
}