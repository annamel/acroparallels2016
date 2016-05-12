//
// Created by kir on 02.05.16.
//

#include "../hash_table.h"
#include <sys/mman.h>

struct chunk_t {
    void *mem;
    ssize_t mem_size;
    off_t mem_offset;
    ssize_t ref_cnt;
};

struct complex_key_t {
    ssize_t size;
    off_t offset;
};

size_t hash_func(const size_t key);
bool cmp_func(const size_t key_a, const size_t key_b);
void destruct_internals_func(struct hash_list_t *hash_list);
bool complex_key_is_ok(struct complex_key_t *complex_key);

static chunk_t *chunk_create(off_t offset, size_t size) {
    assert(size > 0);
    assert(offset >= 0);

    chunk_t *chunk = (chunk_t *)malloc(sizeof(chunk_t));
    assert(chunk);

    chunk->ref_cnt = 1;
    chunk->mem_offset = offset;
    chunk->mem_size = size;
    chunk->mem = NULL;

    return chunk;
}

static complex_key_t *complex_key_clone(const complex_key_t *complex_key) {
    assert(complex_key);

    complex_key_t *complex_key_copy = (complex_key_t *)malloc(sizeof(complex_key_t));
    assert(complex_key_copy);

    memcpy(complex_key_copy, complex_key, sizeof(complex_key_t));

    return complex_key_copy;
}

int main() {
    struct hash_t ht;
    hash_init(&ht, 100, hash_func, cmp_func, destruct_internals_func);

    size_t Page_size = 4096;

    for (int i = 0; i < 10000; i++) {
        complex_key_t key;
        key.offset = Page_size * i;
        key.size = Page_size;

        chunk_t *chunk2ht = chunk_create(key.offset, (size_t)key.size);
        complex_key_t *key2ht = complex_key_clone(&key);

        hash_add_data(&ht, (size_t) key2ht, chunk2ht);
    }

    for (int i = 0; i < 10000; i++) {
        complex_key_t key;
        key.offset = Page_size * i;
        key.size = Page_size;

        assert(hash_find(&ht, (size_t)&key) != NULL);
    }

    for (int i = 0; i < 10000; i++) {
        complex_key_t key;
        key.offset = Page_size * i;
        key.size = Page_size;

        assert(hash_delete_data(&ht, (size_t)&key) == 0);
    }

    hash_fini(&ht);
    return 0;
}

size_t hash_func(const size_t key) {
    complex_key_t *ck = (complex_key_t *)key;
    assert(ck);
    assert(complex_key_is_ok(ck));

    uint64_t single_key = (uint64_t)(ck->offset ^ (size_t)ck->size);

    // MurmerHash3 hash function
    single_key ^= single_key >> 33;
    single_key *= 0xff51afd7ed558ccd;
    single_key ^= single_key >> 33;
    single_key *= 0xc4ceb9fe1a85ec53;
    single_key ^= single_key >> 33;

    return single_key;
}

bool cmp_func(const size_t key_a, const size_t key_b) {
    complex_key_t *ck_a = (complex_key_t *)key_a;
    assert(ck_a);
    assert(complex_key_is_ok(ck_a));

    complex_key_t *ck_b = (complex_key_t *)key_b;
    assert(ck_b);
    assert(complex_key_is_ok(ck_b));

    return (ck_a->offset == ck_b->offset) && (ck_a->size == ck_b->size);
}

void destruct_internals_func(hash_list_t *hash_list) {
    assert(hash_list);
    assert(complex_key_is_ok((complex_key_t *)hash_list->key));

#ifndef NDEBUG
    ((complex_key_t *)hash_list->key)->offset = -1;
    ((complex_key_t *)hash_list->key)->size = -1;
#endif //NDEBUG

    free((void *)hash_list->key); //complex_key_t

    chunk_t *chunk = (chunk_t *)hash_list->data;
    assert(chunk);
    //munmap(chunk->mem, (size_t)chunk->mem_size);

#ifndef NDEBUG
    chunk->mem = NULL;
    chunk->mem_offset = -1;
    chunk->mem_size = -1;
    chunk->ref_cnt = -1;
#endif //NDEBUG

    free(hash_list->data); //chunk_t
}

bool complex_key_is_ok(struct complex_key_t *complex_key) {
    assert(complex_key);
    return (complex_key->offset >= 0) && (complex_key->size >= 0);
}