//
// Created by kir on 25.02.16.
//

#define DEBUG 4

#include "hash_table.h"
#include "logger.h"

//TODO: fit in interval stretagy
//TODO: so on

struct hash_t *hash_construct(size_t hash_size,
                              hash_func_t *hash_func,
                              hash_cmp_func_t *hash_cmp_func,
                              hash_data_destruct_func_t *hash_data_destruct_func,
                              size_t list_data_size) {
    LOG_DEBUG("Called hash_construct(hash_size = %d, hash_func = [%p], "
                     "hash_cmp_func = [%p], list_data_size = %d).\n",
             hash_size, hash_func, hash_cmp_func, list_data_size);

    assert(hash_func);
    assert(hash_cmp_func);
    assert(hash_size > 0);
    assert(list_data_size > 0);

    struct hash_t *hash = (struct hash_t *)malloc(sizeof(struct hash_t));
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
    hash->hash_cmp_func = hash_cmp_func;
    hash->hash_data_destruct_func = hash_data_destruct_func;
    hash->list_data_size = list_data_size;

    LOG_DEBUG("hash_construct: hash constructed and inited, return hash = [%p].\n", hash);
    return hash;
}

void hash_destruct(struct hash_t *hash) {
    LOG_DEBUG("Called hash_destruct( hash = [%p] ).\n", hash);
    assert(hash);

    hash_data_destruct_func_t *data_destruct_func = hash->hash_data_destruct_func;

    for (unsigned i = 0; i < hash->table_size; i++) {
        struct hash_list_t *hash_elem_ptr = hash->table[i];

        while (hash_elem_ptr != NULL) {
            struct hash_list_t *next_elem_ptr = hash_elem_ptr->next;
            if (hash_elem_ptr->data) {
                if (data_destruct_func)
                    data_destruct_func(hash_elem_ptr->data);
                free(hash_elem_ptr->data);
            } else {
                LOG_WARN("hash_cmp_func_t: hash_elem_ptr->data is NULL!.\n", NULL);
            }

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
        if (!hash->hash_cmp_func(cmp_identity, hash_elem_ptr->cmp_identity)) {
            LOG_DEBUG("hash_find: value IS exists [%p].\n", hash_elem_ptr);
            return hash_elem_ptr;
        }
    }

    LOG_DEBUG("hash_find: value is NOT exists..\n", NULL);
    return NULL;
}

struct hash_list_t *hash_add(const struct hash_t *hash, const size_t cmp_identity, const void *data) {
    LOG_DEBUG("Called hash_add( hash = [%p], cmp_identity = [%p], data = [%p] ).\n",
              hash, cmp_identity, data);
    assert(hash);

    size_t hash_key = 0;
    struct hash_list_t *hash_elem_ptr = NULL;

    if (!(hash_elem_ptr = hash_find(hash, cmp_identity))) {
        LOG_DEBUG("hash_add: create new node in hash table.\n", NULL);

        hash_elem_ptr = (struct hash_list_t *)malloc(sizeof(struct hash_list_t));
        if (!hash_elem_ptr) {
            LOG_ERROR("hash_add: failed allocate memory for storing data!.\n", NULL);
            return NULL;
        }

        hash_elem_ptr->cmp_identity = cmp_identity;
        hash_elem_ptr->data = calloc(1, hash->list_data_size);

        if (data)
            memcpy(hash_elem_ptr->data, data, hash->list_data_size);

        hash_key = hash->hash_func(cmp_identity) % hash->table_size;
        hash_elem_ptr->next = hash->table[hash_key];

        hash->table[hash_key] = hash_elem_ptr;
    }

    LOG_DEBUG("hash_add: return [%p].\n", hash_elem_ptr);
    return hash_elem_ptr;
}