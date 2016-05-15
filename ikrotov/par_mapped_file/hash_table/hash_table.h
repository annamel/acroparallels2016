#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include <inttypes.h>
typedef uint32_t hkey_t;
typedef void * hvalue_t;
typedef hkey_t hfunc_t(const hvalue_t,  uint32_t);
typedef struct item item_t;
typedef struct hash_table htable_t;
// default hash function values
#define HASH_CONST_1 0x01000193 
#define HASH_CONST_2 0x811C9DC5 
#ifdef _MSC_VER
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

struct item
{
    hkey_t key;
    hvalue_t value;
    item_t *next;
    item_t *prev;
};

struct hash_table
{
    int size;
    hfunc_t *hash_func;
    item_t **table;
};

htable_t *ht_init(const int size, hfunc_t *hash_func);
int ht_deinit(htable_t *ht);
int ht_add_item(htable_t *ht, hkey_t key, hvalue_t value);
int ht_find_by_key(htable_t *ht, const hkey_t key, item_t **item);
int ht_find_by_key_and_value(htable_t *ht, hkey_t key, hvalue_t *value, item_t **item);
int ht_del_item_by_key(htable_t *ht, hkey_t key);
int ht_del_item_by_key_and_value(htable_t *ht, hkey_t key, hvalue_t value);
hkey_t hash_fnv1a(const hvalue_t key, uint32_t hash);
uint32_t fnv1a(unsigned char oneByte, uint32_t hash);

#endif // HASH_TABLE_H
