//
// Created by kir on 25.02.16.
//

#include "hash_table.h"

hash_t *hash_construct(const unsigned hash_size, hash_func_t *hash_func) {
    assert(hash_func);

    hash_t *hash = (hash_t *)malloc(sizeof(hash_t));
    if (!hash)
        return NULL;

    hash->table = (struct list_t **)calloc(hash_size, sizeof(struct list_t *));
    if (!hash->table) {
        free(hash);
        return NULL;
    }

    hash->table_size = hash_size;
    hash->hash_func = hash_func;

    return hash;
}

void hash_destruct(hash_t *hash) {
    assert(hash);

    for (unsigned i = 0; i < hash->table_size; i++) {
        struct list_t *hash_elem_ptr = hash->table[i];

        while (hash_elem_ptr != NULL) {
            struct list_t *next_elem_ptr = hash_elem_ptr->next;
            free (hash_elem_ptr);
            hash_elem_ptr = next_elem_ptr;
        }
    }

    free(hash->table);
    free(hash);
}

static int hash_store_data_cmp(hash_store_type val_a, hash_store_type val_b) {
    return val_b != val_a;
}

struct list_t *hash_find(const hash_t *hash, const hash_store_type req_data) {
    assert(hash);

    struct list_t *hash_elem_ptr = hash->table[hash->hash_func(req_data) % hash->table_size];
    for (; hash_elem_ptr; hash_elem_ptr = hash_elem_ptr->next) {
        if (!hash_store_data_cmp(req_data, hash_elem_ptr->val)) {
            return hash_elem_ptr;
        }
    }

    return NULL;
}

struct list_t *hash_add(const hash_t *hash, const hash_store_type data) {
    assert(hash);

    uint32_t hash_key = 0;
    struct list_t *hash_elem_ptr = NULL;

    if (!(hash_elem_ptr = hash_find(hash, data))) {
        hash_elem_ptr = (struct list_t *)malloc(sizeof(struct list_t));
        if (!hash_elem_ptr)
            return  NULL;

        hash_elem_ptr->val = data;

        hash_key = hash->hash_func(data) % hash->table_size;
        hash_elem_ptr->next = hash->table[hash_key];

        hash->table[hash_key] = hash_elem_ptr;
    }

    return hash_elem_ptr;
}

void hash_dump (const hash_t *hash)
{
    assert (hash);
    printf ("#> HASH DUMP.\n");
    for (unsigned i = 0; i <  hash->table_size; i++)
    {
        struct list_t *temp_pointer = hash->table[i];
        printf ("#>  AREA %d <<[%p]>>\n", i, temp_pointer);
        if (temp_pointer != NULL)
        {
            do {
                printf ("#>    [%p]\t'%d'\t>[%p]\n",
                        temp_pointer, temp_pointer->val,
                        temp_pointer->next);
                temp_pointer = (temp_pointer->next);
            } while (temp_pointer != NULL);
        }
        else printf ("#>   EMPTY!\n");
    }
    printf ("#> END HASH DUMP.\n");
}


