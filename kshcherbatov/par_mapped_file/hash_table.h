//
// Created by kir on 25.02.16.
//

#ifndef HASHTABLE_HASH_TABLE_H
#define HASHTABLE_HASH_TABLE_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>


struct hash_list_t {
    struct hash_list_t *next;
    ssize_t key;
    void *data;
};

typedef size_t hash_func_t(const size_t key);
typedef bool cmp_func_t(const size_t key_a, const size_t key_b);
typedef void destruct_internals_func_t(hash_list_t *hash_list);

struct hash_t {
    struct hash_list_t** table;
    ssize_t table_size;

    hash_func_t *hash_func;
    cmp_func_t *cmp_func;
    destruct_internals_func_t *destruct_internals_func;
};

void hash_init(struct hash_t *hash, size_t hash_size,
               hash_func_t *hash_func, cmp_func_t *cmp_func,
               destruct_internals_func_t *destruct_func);
void hash_fini(struct hash_t *hash);
void *hash_find(const struct hash_t *hash, const size_t key);
struct hash_list_t *hash_add_data(const struct hash_t *hash, const size_t key, void *data);
int hash_delete_data(const struct hash_t *hash, const size_t key);
bool hash_is_ok(const struct hash_t *hash);
bool hash_list_is_ok(struct hash_list_t *hash_list);

#endif //HASHTABLE_HASH_TABLE_H
