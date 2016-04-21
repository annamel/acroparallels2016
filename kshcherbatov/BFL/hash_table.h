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

typedef size_t hash_func_t(const size_t cmp_identity);

struct hash_list_t {
    struct hash_list_t* next;
    size_t cmp_identity;
    void *data;
};

struct hash_t {
    struct hash_list_t** table;
    size_t table_size;

    hash_func_t *hash_func;
};

struct hash_t *hash_construct(size_t hash_size, hash_func_t *hash_func);
void hash_destruct(struct hash_t *hash);
struct hash_list_t *hash_find(const struct hash_t *hash, const size_t cmp_identity);
struct hash_list_t *hash_add(const struct hash_t *hash, const size_t cmp_identity, void *data);

#endif //HASHTABLE_HASH_TABLE_H
