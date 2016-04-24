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
    assert(hash_is_ok(hash));

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
    assert(hash_is_ok(hash));

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

struct hash_list_t *hash_node_construct(const struct hash_t *hash, const size_t cmp_identity, void *data) {
    LOG_DEBUG("Called hash_node_construct( hash = [%p], cmp_identity = [%p], data = [%p] ).\n",
              hash, cmp_identity, data);
    assert(hash);
    assert(hash_is_ok(hash));

    struct hash_list_t *hash_node = (struct hash_list_t *)malloc(sizeof(struct hash_list_t));
    if (!hash_node) {
        LOG_ERROR("hash_node_construct: failed allocate memory for storing data!.\n", NULL);
        return NULL;
    }

    hash_node->cmp_identity = cmp_identity;
    hash_node->data = data;

    hash_node->next = NULL;
    hash_node->prev = NULL;

    return hash_node;
}

struct hash_list_t *hash_add_data(const struct hash_t *hash, const size_t cmp_identity, void *data) {
    LOG_DEBUG("Called hash_add_data( hash = [%p], cmp_identity = [%p], data = [%p] ).\n",
              hash, cmp_identity, data);
    assert(hash);
    assert(hash_is_ok(hash));

    struct hash_list_t *hash_elem_ptr = NULL;

    if (!(hash_elem_ptr = hash_find(hash, cmp_identity))) {
        LOG_DEBUG("hash_add_data: create new node in hash table.\n", NULL);

        hash_elem_ptr = hash_node_construct(hash, cmp_identity, data);
        if (!hash_elem_ptr)
            return NULL;

        size_t hash_key = hash->hash_func(cmp_identity) % hash->table_size;

        hash_elem_ptr->next = hash->table[hash_key];
        if (hash->table[hash_key])
            hash->table[hash_key]->prev = hash_elem_ptr;

        assert(hash_elem_ptr->prev == NULL);

        hash->table[hash_key] = hash_elem_ptr;
    }

    LOG_DEBUG("hash_add_data: return [%p].\n", hash_elem_ptr);
    return hash_elem_ptr;
}

struct hash_list_t *hash_add_node(const struct hash_t *hash, struct hash_list_t *hash_list) {
    LOG_DEBUG("Called hash_add_node( hash = [%p], hash_list = [%p], cmp_identity = [%p] ).\n",
              hash, hash_list);
    assert(hash);
    assert(hash_list);
    assert(hash_is_ok(hash));

    size_t hash_key = hash->hash_func(hash_list->cmp_identity) % hash->table_size;

    hash_list->next = hash->table[hash_key];
    if (hash->table[hash_key])
        hash->table[hash_key]->prev = hash_list;

    assert(hash_list->prev == NULL);

    hash->table[hash_key] = hash_list;

    LOG_DEBUG("hash_add_node: return [%p].\n", hash_list);
    return hash_list;
}

void hash_delete_node(const struct hash_t *hash, struct hash_list_t *hash_list) {
    LOG_DEBUG("Called hash_delete_node( hash = [%p], hash_list = [%p] ).\n",
              hash, hash_list);
    assert(hash);
    assert(hash_list);
    assert(hash_is_ok(hash));

    if (hash_list->prev)
        hash_list->prev->next = hash_list->next;

    if (hash_list->next)
        hash_list->next->prev = hash_list->prev;

    size_t hash_key = hash->hash_func(hash_list->cmp_identity) % hash->table_size;
    assert(hash->table[hash_key]);

    if (hash->table[hash_key] == hash_list) {
        hash->table[hash_key] = hash_list->next;
    }

    free((void *)hash_list);
}

bool hash_is_ok(const struct hash_t *hash) {
    assert(hash);
    return hash->table && hash->hash_func && (hash->table_size > 0);
}