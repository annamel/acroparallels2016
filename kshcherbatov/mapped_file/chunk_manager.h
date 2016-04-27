//
// Created by kir on 24.04.16.
//

#ifndef BFL_MAPPED_FILE_INTERNALS_H
#define BFL_MAPPED_FILE_INTERNALS_H

#include "hash_table.h"

struct ht_node_ref_t {
    ht_node_ref_t *next;
    hash_list_t *hash_list;
};

struct chunk_t {
    void *mapped_area;
    hash_list_t *hash_list;

    off_t pa_offset;
    ssize_t mapped_area_size;
    ssize_t reference_counter;

    bool in_cache;

    ht_node_ref_t *ht_node;
};

struct mapped_file_t {
    int fd;
    ssize_t file_size;
    ssize_t chunk_std_size;
    struct hash_t *chunk_ptr_ht;
    
    chunk_t *cache;

    ssize_t chunk_pool_size;
    struct chunk_t *chunk_pool;
    ssize_t chunk_pool_w_idx;
};

bool mapped_file_is_ok(mapped_file_t *mapped_file);
bool chunk_is_empty(chunk_t *chunk);

mapped_file_t *mapped_file_construct(const char *filename, size_t std_chunk_size);
void mapped_file_destruct(mapped_file_t *mapped_file);

void *chunk_mem_acquire(struct mapped_file_t *mapped_file, off_t offset, size_t size,
                        bool choose_biggest, size_t *read_size, chunk_t **associated_chunk);

void chunk_mem_unacquire(struct chunk_t *chunk);

ssize_t mapped_file_data_memcpy(struct mapped_file_t *mapped_file,
                                off_t offset, size_t size, void *buf, bool write_mode);

ssize_t mapped_file_data_memcpy(struct mapped_file_t *mapped_file,
                                off_t offset, size_t size, void *buf, bool write_mode);

#endif //BFL_MAPPED_FILE_INTERNALS_H
