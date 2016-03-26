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

// to change stored value type edit hash_store_type typedef; hash_store_data_cmp; hash_function

typedef uint64_t hash_func_t(const void *cmp_identity);
typedef int hash_cmp_func_t(const void *cmp_identity_a, const void *cmp_identity_b);

struct list_t {
    struct list_t* next;
    void *cmp_identity;
    void *data;
};

struct hash_t {
    struct list_t** table;
    unsigned table_size;
    unsigned list_data_size;

    hash_func_t *hash_func;
    hash_cmp_func_t *hash_cmp_func;

};

struct hash_t *hash_construct(unsigned hash_size, hash_func_t *hash_func, 
	hash_cmp_func_t *hash_cmp_func, unsigned list_data_size);
void hash_destruct(hash_t *hash);
struct list_t *hash_find(const hash_t *hash, const void *cmp_identity);
struct list_t *hash_add(const hash_t *hash, const void *cmp_identity, const void *data);

#endif //HASHTABLE_HASH_TABLE_H
