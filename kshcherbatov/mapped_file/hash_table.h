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
    struct hash_list_t *next;
    struct hash_list_t *prev;

    ssize_t cmp_identity;
    void *data;
};

struct hash_t {
    struct hash_list_t** table;
    ssize_t table_size;

    hash_func_t *hash_func;
};

struct hash_t *hash_construct(size_t hash_size, hash_func_t *hash_func);
void hash_destruct(struct hash_t *hash);
bool hash_is_ok(const struct hash_t *hash);
struct hash_list_t *hash_find(const struct hash_t *hash, const size_t cmp_identity);
struct hash_list_t *hash_node_construct(const struct hash_t *hash, const size_t cmp_identity, void *data);
struct hash_list_t *hash_add_data(const struct hash_t *hash, const size_t cmp_identity, void *data);
struct hash_list_t *hash_add_node(const struct hash_t *hash, struct hash_list_t *hash_list);
void hash_delete_node(const struct hash_t *hash, struct hash_list_t *hash_list);

#endif //HASHTABLE_HASH_TABLE_H
