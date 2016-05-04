#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include "hash_funcs.h"
typedef hkey_t hfunc_t(const hvalue_t,  uint32_t);
typedef struct item item_t;
typedef struct hash_table htable_t;

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

int ht_find_by_kav(htable_t *ht, hkey_t key, hvalue_t *value, item_t **item);

int ht_del_item_by_key(htable_t *ht, hkey_t key);

int ht_del_item_by_kav(htable_t *ht, hkey_t key, hvalue_t value);

void print_table(htable_t *ht);

int number_of_items_in_index(htable_t *ht, int index);

int print_list_in_index(htable_t *ht, int index);


#endif // HASH_TABLE_H
