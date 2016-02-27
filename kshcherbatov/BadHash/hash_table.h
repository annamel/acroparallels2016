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

typedef uint32_t hash_store_type;
typedef uint32_t hash_func_t(const hash_store_type data);

struct list_t {
    list_t* next;
    hash_store_type val;
};

typedef struct {
    struct list_t** table;
    unsigned table_size;
    hash_func_t *hash_func;
} hash_t;

hash_t *hash_construct(const unsigned hash_size, hash_func_t *hash_func);
void hash_destruct(hash_t *hash);
struct list_t *hash_find(const hash_t *hash, const hash_store_type req_data);
struct list_t *hash_add(const hash_t *hash, const hash_store_type data);
void hash_dump (const hash_t *hash);

#endif //HASHTABLE_HASH_TABLE_H
