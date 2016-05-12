//
// Created by kir on 25.02.16.
//

// data structure concept from "The C Programming Language" by Brian W. Kernighan, DennisM. Ritchie.

#include "hash_table.h"

void hash_init(struct hash_t *hash, size_t hash_size,
               hash_func_t *hash_func, cmp_func_t *cmp_func, destruct_internals_func_t *destruct_func) {
    assert(hash);
    assert(hash_func);
    assert(cmp_func);
    assert(hash_size > 0);

    hash->table_size = hash_size;
    hash->hash_func = hash_func;
    hash->cmp_func = cmp_func;
    hash->destruct_internals_func = destruct_func;

    hash->table = (struct hash_list_t **)calloc(hash_size, sizeof(struct hash_list_t*));
    assert(hash->table);
}

void hash_fini(struct hash_t *hash) {
    assert(hash);
    assert(hash_is_ok(hash));

    destruct_internals_func_t *destruct_internals_func = hash->destruct_internals_func;

    for (size_t i = 0; i < hash->table_size; i++) {
        struct hash_list_t *hash_elem_ptr = hash->table[i];

        while (hash_elem_ptr) {
            struct hash_list_t *next_elem_ptr = hash_elem_ptr->next;

            assert(hash_list_is_ok(hash_elem_ptr));

            if (destruct_internals_func)
                destruct_internals_func(hash_elem_ptr);

#ifndef NDEBUG
            hash_elem_ptr->key = (size_t)NULL;
            hash_elem_ptr->data = NULL;
            hash_elem_ptr->next = NULL;
#endif //NDEBUG

            free((void *)hash_elem_ptr);

            hash_elem_ptr = next_elem_ptr;
        }
    }

    free((void *)hash->table);

#ifndef NDEBUG
    hash->table = NULL;
    hash->hash_func = NULL;
    hash->cmp_func = NULL;
    hash->destruct_internals_func = NULL;
    hash->table_size = -1;
#endif //NDEBUG
}

void *hash_find(const struct hash_t *hash, const size_t key) {
    assert(hash);
    assert(hash_is_ok(hash));

    size_t hash_table_idx = hash->hash_func(key) % hash->table_size;
    struct hash_list_t *hash_elem_ptr = hash->table[hash_table_idx];

    cmp_func_t *cmp_func = hash->cmp_func;

    for (; hash_elem_ptr; hash_elem_ptr = hash_elem_ptr->next) {
        assert(hash_list_is_ok(hash_elem_ptr));

        if (cmp_func((size_t)hash_elem_ptr->key, key)) {
            return hash_elem_ptr->data;
        }
    }

    return NULL;
}

struct hash_list_t *hash_add_data(const struct hash_t *hash, const size_t key, void *data) {
    assert(hash);
    assert(hash_is_ok(hash));

    cmp_func_t *cmp_func = hash->cmp_func;
    hash_list_t **table = hash->table;

    size_t hash_table_idx = hash->hash_func(key) % hash->table_size;
    struct hash_list_t *hash_elem_ptr = table[hash_table_idx];

    if (!hash_elem_ptr) {
        table[hash_table_idx] = (hash_list_t *)malloc(sizeof(hash_list_t));
        assert(table[hash_table_idx]);

        hash_elem_ptr = table[hash_table_idx];
    } else {
        for (; hash_elem_ptr->next; hash_elem_ptr = hash_elem_ptr->next) {
            if (cmp_func((size_t)hash_elem_ptr->key, key))
                return hash_elem_ptr;
        }

        if (cmp_func((size_t)hash_elem_ptr->key, key))
            return hash_elem_ptr;

        hash_elem_ptr->next = (hash_list_t *)malloc(sizeof(hash_list_t));
        assert(hash_elem_ptr->next);
        hash_elem_ptr = hash_elem_ptr->next;
    }

    hash_elem_ptr->key = key;
    hash_elem_ptr->data = data;
    hash_elem_ptr->next = NULL;

    assert(hash_list_is_ok(hash_elem_ptr));
    return hash_elem_ptr;
}

int hash_delete_data(const struct hash_t *hash, const size_t key) {
    assert(hash);
    assert(hash_is_ok(hash));

    cmp_func_t *cmp_func = hash->cmp_func;
    destruct_internals_func_t *destruct_internals_func = hash->destruct_internals_func;

    size_t hash_table_idx = hash->hash_func(key) % hash->table_size;

    struct hash_list_t *hash_elem_prev_ptr = hash->table[hash_table_idx];
    struct hash_list_t *hash_elem_ptr = hash_elem_prev_ptr ? hash_elem_prev_ptr->next : NULL;

    if (!hash_elem_prev_ptr) {
        assert(!"hash_delete_data: no hash_elem_prev_ptr unexpected situation!");
        return -1;
    }

    if (cmp_func((size_t)hash_elem_prev_ptr->key, key)) {
        // entry node
        assert(hash_list_is_ok(hash_elem_prev_ptr));

        if (destruct_internals_func)
            destruct_internals_func(hash_elem_prev_ptr);

        free(hash_elem_prev_ptr);
        hash->table[hash_table_idx] = hash_elem_ptr;

        return 0;
    }

    // non-entry node
    for (; hash_elem_ptr; hash_elem_prev_ptr = hash_elem_ptr, hash_elem_ptr = hash_elem_ptr->next) {
        if (cmp_func((size_t)hash_elem_ptr->key, key)) {
            hash_elem_prev_ptr->next = hash_elem_ptr->next;

            assert(hash_list_is_ok(hash_elem_ptr));

            if (destruct_internals_func)
                destruct_internals_func(hash_elem_ptr);

#ifndef NDEBUG
            hash_elem_ptr->key = -0xDEAD;
            hash_elem_ptr->data = NULL;
            hash_elem_ptr->next = NULL;
#endif //NDEBUG

            free(hash_elem_ptr);

            return 0;
        }
    }
}

bool hash_is_ok(const struct hash_t *hash) {
    assert(hash);
    return hash->table && hash->hash_func && hash->cmp_func && (hash->table_size > 0);
}


bool hash_list_is_ok(struct hash_list_t *hash_list) {
    assert(hash_list);
    return (hash_list->key != -0xDEAD);
}