//
// Created by kir on 25.02.16.
//

#include "hash_table.h"
#include "logger.h"

struct hash_t *hash_construct(unsigned hash_size, hash_func_t *hash_func,
                              hash_cmp_func_t *hash_cmp_func, unsigned list_data_size) {
    init_log_system("log.txt");
    LOG_DEBUG("Called hash_construct(hash_size = %d, hash_func = [%p], "
                     "hash_cmp_func = [%p], list_data_size = %d)\n",
             hash_size, hash_func, hash_cmp_func, list_data_size);

    assert(hash_func);
    assert(hash_cmp_func);
    assert(hash_size > 0);
    assert(list_data_size > 0);

    struct hash_t *hash = (struct hash_t *)malloc(sizeof(struct hash_t));
    if (!hash) {
        LOG_FATAL("hash_construct: failed allocate memory for hash\n");
        return NULL;
    }

    hash->table = (struct list_t **)calloc(hash_size, sizeof(struct list_t *));
    if (!hash->table) {
        LOG_FATAL("hash_construct: failed allocate memory for hash list\n");
        free((void *)hash);
        return NULL;
    }

    hash->table_size = hash_size;
    hash->hash_func = hash_func;
    hash->hash_cmp_func = hash_cmp_func;
    hash->list_data_size = list_data_size;

    LOG_DEBUG("hash_construct: hash constructed and inited, return hash = [%p]\n", hash);
    return hash;
}

void hash_destruct(struct hash_t *hash) {
    LOG_DEBUG("Called hash_destruct( hash = [%p] )\n", hash);

    assert(hash);

    for (unsigned i = 0; i < hash->table_size; i++) {
        struct list_t *hash_elem_ptr = hash->table[i];

        while (hash_elem_ptr != NULL) {
            struct list_t *next_elem_ptr = hash_elem_ptr->next;
            if (hash_elem_ptr->data)
                free((void *)hash_elem_ptr->data);
            else
                LOG_WARN("hash_cmp_func_t: hash_elem_ptr->data is NULL!\n");

            free((void *)hash_elem_ptr);
            hash_elem_ptr = next_elem_ptr;
        }
    }

    free((void *)hash->table);
    free((void *)hash);

    LOG_DEBUG("hash_destruct: destruction finished\n");
    deinit_log_system();
}


struct list_t *hash_find(const hash_t *hash, const void *cmp_identity) {
    LOG_DEBUG("Called hash_find( hash = [%p], cmp_identity = [%p] )\n", hash, cmp_identity);
    assert(hash);

    struct list_t *hash_elem_ptr = hash->table[hash->hash_func(cmp_identity) % hash->table_size];
    for (; hash_elem_ptr; hash_elem_ptr = hash_elem_ptr->next) {
        if (!hash->hash_cmp_func(cmp_identity, hash_elem_ptr->cmp_identity)) {
            LOG_DEBUG("hash_find: value exists [%p]!\n", hash_elem_ptr);
            return hash_elem_ptr;
        }
    }

    LOG_DEBUG("hash_find: value is NOT exists.\n");
    return NULL;
}

struct list_t *hash_add(const hash_t *hash, const void *cmp_identity, const void *data) {
    LOG_DEBUG("Called hash_add( hash = [%p], cmp_identity = [%p], data = [%p] )\n",
              hash, cmp_identity, data);
    assert(hash);

    uint32_t hash_key = 0;
    struct list_t *hash_elem_ptr = NULL;

    if (!(hash_elem_ptr = hash_find(hash, cmp_identity))) {
        LOG_DEBUG("hash_add: create now node in hash table\n");

        hash_elem_ptr = (struct list_t *)malloc(sizeof(struct list_t));
        if (!hash_elem_ptr) {
            LOG_FATAL("hash_add: failed allocate memory for storing data!\n");
            return NULL;
        }

        hash_elem_ptr->cmp_identity = (void *)cmp_identity;
        hash_elem_ptr->data = malloc(hash->list_data_size);
        memcpy((void *)hash_elem_ptr->data, data, hash->list_data_size);

        hash_key = hash->hash_func(data) % hash->table_size;
        hash_elem_ptr->next = hash->table[hash_key];

        hash->table[hash_key] = hash_elem_ptr;
    }

    LOG_DEBUG("hash_add: return [%p]\n", hash_elem_ptr);
    return hash_elem_ptr;
}