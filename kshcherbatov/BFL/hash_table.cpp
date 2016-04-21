//
// Created by kir on 25.02.16.
//

#define DEBUG 4

#include "hash_table.h"
#include "logger.h"

struct hash_t *hash_construct(size_t hash_size, hash_func_t *hash_func) {
    LOG_DEBUG("Called hash_construct(hash_size = %d, hash_func = [%p])\n", hash_size, hash_func);

    assert(hash_func);
    assert(hash_size > 0);

    struct hash_t *hash = (struct hash_t *)calloc(1, sizeof(struct hash_t));
    if (!hash) {
        LOG_ERROR("hash_construct: failed allocate memory for hash.\n", NULL);
        return NULL;
    }

    hash->table = (struct hash_list_t **)calloc(hash_size, sizeof(struct hash_list_t *));
    if (!hash->table) {
        LOG_ERROR("hash_construct: failed allocate memory for hash list.\n", NULL);
        free((void *)hash);
        return NULL;
    }

    hash->table_size = hash_size;
    hash->hash_func = hash_func;

    LOG_DEBUG("hash_construct: hash constructed and inited, return hash = [%p].\n", hash);
    return hash;
}

void hash_destruct(struct hash_t *hash) {
    LOG_DEBUG("Called hash_destruct( hash = [%p] ).\n", hash);
    assert(hash);


    for (unsigned i = 0; i < hash->table_size; i++) {
        struct hash_list_t *hash_elem_ptr = hash->table[i];

        while (hash_elem_ptr != NULL) {
            struct hash_list_t *next_elem_ptr = hash_elem_ptr->next;
            free((void *)hash_elem_ptr);
            hash_elem_ptr = next_elem_ptr;
        }
    }

    free((void *)hash->table);
    free((void *)hash);

    LOG_DEBUG("hash_destruct: destruction finished.\n", NULL);
}

struct hash_list_t *hash_find(const struct hash_t *hash, const size_t cmp_identity) {
    LOG_DEBUG("Called hash_find( hash = [%p], cmp_identity = [%p] ).\n", hash, cmp_identity);
    assert(hash);

    struct hash_list_t *hash_elem_ptr = hash->table[hash->hash_func(cmp_identity) % hash->table_size];
    for (; hash_elem_ptr; hash_elem_ptr = hash_elem_ptr->next) {
        if (cmp_identity == hash_elem_ptr->cmp_identity) {
            LOG_DEBUG("hash_find: value IS exists [%p].\n", hash_elem_ptr);
            return hash_elem_ptr;
        }
    }

    LOG_DEBUG("hash_find: value is NOT exists..\n", NULL);
    return NULL;
}

struct hash_list_t *hash_add(const struct hash_t *hash, const size_t cmp_identity, void *data) {
    LOG_DEBUG("Called hash_add( hash = [%p], cmp_identity = [%p], data = [%p] ).\n",
              hash, cmp_identity, data);
    assert(hash);

    struct hash_list_t *hash_elem_ptr = NULL;

    if (!(hash_elem_ptr = hash_find(hash, cmp_identity))) {
        LOG_DEBUG("hash_add: create new node in hash table.\n", NULL);

        hash_elem_ptr = (struct hash_list_t *)malloc(sizeof(struct hash_list_t));
        if (!hash_elem_ptr) {
            LOG_ERROR("hash_add: failed allocate memory for storing data!.\n", NULL);
            return NULL;
        }

        hash_elem_ptr->cmp_identity = cmp_identity;
        hash_elem_ptr->data = data;

        size_t hash_key = hash->hash_func(cmp_identity) % hash->table_size;
        hash_elem_ptr->next = hash->table[hash_key];

        hash->table[hash_key] = hash_elem_ptr;
    }

    LOG_DEBUG("hash_add: return [%p].\n", hash_elem_ptr);
    return hash_elem_ptr;
}