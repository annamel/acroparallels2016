//
// Created by kir on 24.04.16.
//

#include "logger.h"
#include "chunk_manager.h"
#include "errno.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


size_t hash_func(const size_t cmp_identity);
struct ht_node_ref_t *ht_node_construct(hash_list_t *hash_list);
static off_t pa_offset(off_t offset);
int chunk_map(struct chunk_t *chunk, int fd, off_t offset, size_t size);
void chunk_unmap(struct chunk_t *chunk);
ssize_t chunk_pool_place_idx_find(struct mapped_file_t *mapped_file);
struct hash_list_t *chunk_approp_hash_list(struct hash_t *hash, size_t offset, size_t size, bool choose_biggest);
struct chunk_t *chunk_get(struct mapped_file_t *mapped_file, off_t offset, size_t size, bool choose_biggest);


bool mapped_file_is_ok(mapped_file_t *mapped_file) {
    assert(mapped_file);
    return mapped_file->chunk_ptr_ht && hash_is_ok(mapped_file->chunk_ptr_ht) && mapped_file->chunk_pool
           && (mapped_file->chunk_pool_size > 0) && (mapped_file->fd >= 0) && (mapped_file->file_size >= 0)
           && (mapped_file->chunk_std_size == (ssize_t)pa_offset(mapped_file->chunk_std_size));
}

bool chunk_is_empty(chunk_t *chunk) {
    assert(chunk);
    return !chunk->mapped_area && chunk->mapped_area_size <= 0;
}

mapped_file_t *mapped_file_construct(const char *filename, size_t std_chunk_size) {
    LOG_DEBUG("Called mapped_file_construct (pathname = %s, std_chunk_size = %u\n", filename, std_chunk_size=);

    assert(filename);
    assert(std_chunk_size > 0);

    struct mapped_file_t *mapped_file = (struct mapped_file_t *)calloc(1, sizeof(struct mapped_file_t));

    int fd;
    if ((fd = open(filename, O_RDWR)) < 0) {
        LOG_ERROR("mapped_file_construct: open: failed open file.\n", NULL);
        free((void *)mapped_file);
        return NULL;
    }
    mapped_file->fd = fd;

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        LOG_ERROR("mapped_file_construct: fstat: failed obtaining file info using fstat.\n", NULL);
        close(fd);
        free((void *)mapped_file);
        return NULL;
    }
    mapped_file->file_size = (size_t) sb.st_size;
    size_t hash_table_size = 6*(size_t) sb.st_size / (size_t)sysconf(_SC_PAGE_SIZE);

    std_chunk_size = (size_t)pa_offset(std_chunk_size);
    if (!std_chunk_size)
        std_chunk_size = (size_t)sysconf(_SC_PAGE_SIZE);
    mapped_file->chunk_std_size = std_chunk_size;
    size_t chunk_pool_size = 4*(mapped_file->file_size / std_chunk_size);

    mapped_file->chunk_ptr_ht = hash_construct(hash_table_size, hash_func);
    if (!mapped_file->chunk_ptr_ht) {
        LOG_ERROR("mapped_file_construct: hash_construct: failed construct hash table.\n", NULL);
        close(fd);
        free((void *)mapped_file);
        return NULL;
    }

    mapped_file->cache = NULL;
    
    mapped_file->chunk_pool_size = chunk_pool_size;
    mapped_file->chunk_pool_w_idx = 0;
    mapped_file->chunk_pool = (struct chunk_t *) calloc(chunk_pool_size, sizeof(struct chunk_t));
    if (!mapped_file->chunk_pool) {
        LOG_ERROR("mapped_file_construct: failed allocate momory for chunk_pool\n", NULL);
        hash_destruct(mapped_file->chunk_ptr_ht);
        close(fd);
        free((void *)mapped_file);
        return NULL;
    }

    return mapped_file;
}

void mapped_file_destruct(mapped_file_t *mapped_file) {
    LOG_DEBUG("Called mapped_file_destruct (mapped_file = [%p] ).\n", mapped_file);
    assert(mapped_file);
    assert(mapped_file_is_ok(mapped_file));

    for (ssize_t i = 0; i < mapped_file->chunk_pool_size; i++) {
        chunk_unmap(&(mapped_file->chunk_pool[i]));
#ifdef DEBUG
        mapped_file->chunk_pool[i].hash_list = NULL;
        mapped_file->chunk_pool[i].ht_node = NULL;
        mapped_file->chunk_pool[i].mapped_area = NULL;
        mapped_file->chunk_pool[i].pa_offset = (off_t)(-1);
        mapped_file->chunk_pool[i].mapped_area_size = -1;
        mapped_file->chunk_pool[i].reference_counter = -1;
#endif
    }

    free((void *) mapped_file->chunk_pool);
    hash_destruct(mapped_file->chunk_ptr_ht);
    close(mapped_file->fd);

#ifdef DEBUG
    mapped_file->chunk_pool = NULL;
    mapped_file->chunk_ptr_ht = NULL;
    mapped_file->chunk_pool_size = -1;
    mapped_file->chunk_pool_w_idx = -1;
    mapped_file->chunk_std_size = -1;
    mapped_file->file_size = -1;
    mapped_file->fd = -1;
    mapped_file->cache = NULL;
#endif

    free((void *)mapped_file);
#ifdef DEBUG
    mapped_file = NULL;
#endif
}

void *chunk_mem_acquire(struct mapped_file_t *mapped_file, off_t offset, size_t size,
                        bool choose_biggest, size_t *read_size, chunk_t **associated_chunk) {
    LOG_DEBUG("Called chunk_mem_acquire (mapped_file = [%p], offset = %u, size = %dm choose_biggest = %d).\n",
              mapped_file, offset, size, choose_biggest);
    assert(mapped_file);
    assert(mapped_file_is_ok(mapped_file));

    if (offset > (off_t)mapped_file->file_size) {
        LOG_WARN("chunk_mem_acquire: offset is out of the file.\n", NULL);
        return NULL;
    }
    if (size == 0) {
        size = (size_t)mapped_file->chunk_std_size;
    }

    off_t chunk_pa_offset = pa_offset(offset);
    size_t mem_size;
    if ((ssize_t)(offset + size) > mapped_file->file_size) {
        size = (size_t)mapped_file->file_size - offset;
        mem_size = size;
    } else {
        mem_size = (size_t)pa_offset(size + (offset - chunk_pa_offset) + sysconf(_SC_PAGE_SIZE));
        if ((ssize_t)(chunk_pa_offset + mem_size) > mapped_file->file_size) {
            mem_size = (size_t)mapped_file->file_size - chunk_pa_offset;
        }
    }

    if (mem_size == 0)
        return NULL;

    if (mapped_file->cache) {
        off_t cache_offset = mapped_file->cache->pa_offset;
        size_t cache_mapped_area_size = (size_t)mapped_file->cache->mapped_area_size;

        if (offset > cache_offset && offset+size < cache_offset + cache_mapped_area_size) {
            *read_size = cache_mapped_area_size - (offset - cache_offset);
            *associated_chunk = mapped_file->cache;

            LOG_DEBUG("Exit chunk_mem_acquire: cache fit!\n", NULL);
            return (void *) ((size_t)mapped_file->cache->mapped_area + (offset - cache_offset));
        }
    }

    chunk_t *chunk = chunk_get(mapped_file, chunk_pa_offset, mem_size, choose_biggest);
    if (!chunk) {
        LOG_ERROR("chunk_mem_acquire: failed get chunk in pool\n\n", NULL);
        *read_size = 0;
        *associated_chunk = NULL;
        return NULL;
    }

    if (!chunk->mapped_area) {
        assert(chunk_is_empty(chunk));
        LOG_DEBUG("chunk_mem_acquire: opening the gate in chunk.\n", NULL);

        chunk->reference_counter = 0;

        if (chunk_map(chunk, mapped_file->fd, chunk_pa_offset, mem_size) < 0) {
            LOG_ERROR("chunk_mem_acquire: failed chunk_map\n", NULL);
            return NULL;
        }

        assert(!chunk->ht_node);
        size_t off_iterator = (size_t)(chunk_pa_offset + sysconf(_SC_PAGE_SIZE));
        while (off_iterator < chunk_pa_offset + mem_size) {
            hash_list_t *chunk_hash_list_node = hash_node_construct(mapped_file->chunk_ptr_ht, off_iterator, chunk);
            assert(chunk_hash_list_node);

            hash_add_node(mapped_file->chunk_ptr_ht, chunk_hash_list_node);

            ht_node_ref_t *ht_node = ht_node_construct(chunk_hash_list_node);
            assert(ht_node);

            ht_node->next = chunk->ht_node;
            chunk->ht_node = ht_node;

            off_iterator += sysconf(_SC_PAGE_SIZE);
        }
    } else {
        assert(!chunk_is_empty(chunk));
        LOG_DEBUG("chunk_mem_acquire: loaded chunk [%p]: av. size %d, req_size %d\n",
                  chunk, chunk->mapped_area_size - (offset - chunk->pa_offset), size);
        assert(chunk->mapped_area_size - (offset - chunk->pa_offset) >= (ssize_t)size);
        LOG_DEBUG("chunk_mem_acquire: chunk already has a gate.\n", NULL);
    }

    chunk->reference_counter++;

    *read_size = (size_t)chunk->mapped_area_size - (offset - chunk->pa_offset);
    *associated_chunk = chunk;
    mapped_file->cache = chunk;

    LOG_DEBUG("Exit chunk_mem_acquire: obtained memory region pointer, size = %u\n\n", *read_size);

    return (void *) ((size_t) chunk->mapped_area + (offset - chunk->pa_offset));
}

void chunk_mem_unacquire(struct chunk_t *chunk) {
    LOG_DEBUG("Called chunk_mem_unacquire (chunk = [%p]).\n", chunk);
    assert(chunk);
    assert(!chunk_is_empty(chunk));

    if (chunk->reference_counter > 0)
        chunk->reference_counter--;

    LOG_DEBUG("\n\n", NULL);
}

ssize_t mapped_file_data_memcpy(struct mapped_file_t *mapped_file, off_t offset,
                                size_t size, void *buf, bool write_mode) {
    LOG_DEBUG("Called mapped_file_data_memcpy (mapped_file = [%p],"
                      " offset = [%u], size = [%u], buf  = [%p], write_mode = %d).\n",
              mapped_file, offset, size, buf, write_mode);

    assert(mapped_file);
    assert(mapped_file_is_ok(mapped_file));
    assert(size > 0);
    assert(buf);

    off_t end_offset = offset + size;
    ssize_t bytes_processed = 0;
    bool should_exit = false;

    while (offset < end_offset) {
        size_t read_size;
        chunk_t *chunk;
        size_t req_size = (ssize_t)size < mapped_file->chunk_std_size ? size : 0;

        void *data_ptr = chunk_mem_acquire(mapped_file, offset, req_size, true, &read_size, &chunk);
        if (!data_ptr)
            break;

        size_t bytes_to_process = size - bytes_processed;

        if (bytes_to_process <= read_size) {
            should_exit = true;
        } else {
            bytes_to_process = read_size;
        }

        if (write_mode) {
            memcpy(data_ptr, (void *)((size_t)buf + bytes_processed), bytes_to_process);
        } else {
            memcpy((void *)((size_t)buf + bytes_processed), data_ptr, bytes_to_process);
        }

        bytes_processed += bytes_to_process;

        chunk_mem_unacquire(chunk);

        offset += read_size;
        if (offset >= (off_t)mapped_file->file_size || should_exit)
            break;

        offset = pa_offset(offset);
    }

    LOG_DEBUG("Exit mapped_file_data_memcpy: processed bytes = %d\n\n", bytes_processed);
    return bytes_processed;
}


size_t hash_func(const size_t cmp_identity) {
    uint64_t key = (uint64_t) cmp_identity;

    key ^= key >> 33;
    key *= 0xff51afd7ed558ccd;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53;
    key ^= key >> 33;

    return key;
}


struct ht_node_ref_t *ht_node_construct(hash_list_t *hash_list) {
    ht_node_ref_t *ht_node = (ht_node_ref_t *)calloc(1, sizeof(ht_node_ref_t));
    ht_node->hash_list = hash_list;
    return ht_node;
}

static off_t pa_offset(off_t offset) {
    return (off_t) (offset & ~(sysconf(_SC_PAGE_SIZE) - 1));
}

ssize_t chunk_pool_place_idx_find(struct mapped_file_t *mapped_file) {
    assert(mapped_file);
    assert(mapped_file_is_ok(mapped_file));

    size_t chunk_pool_size = (size_t)mapped_file->chunk_pool_size;
    size_t chunk_pool_w_idx = (size_t)mapped_file->chunk_pool_w_idx;
    chunk_t *chunk_pool = mapped_file->chunk_pool;

    assert(chunk_pool_size > 0);
    assert(chunk_pool);

    ssize_t chunk_pool_w_idx_new = -1;

    for (size_t w_idx = chunk_pool_w_idx; w_idx < chunk_pool_size + chunk_pool_w_idx; w_idx++) {
        size_t chunk_pool_elem_idx = w_idx % chunk_pool_size;

        chunk_t *chunk = &(chunk_pool[chunk_pool_elem_idx]);
        assert(chunk);

        if (!chunk->mapped_area || chunk->reference_counter == 0) {
            chunk_pool_w_idx_new = chunk_pool_elem_idx;
            break;
        }
    }

    LOG_DEBUG("Exit chunk_pool_place_idx_find: return %d\n", chunk_pool_w_idx_new);
    return chunk_pool_w_idx_new;
}

struct hash_list_t *chunk_approp_hash_list(struct hash_t *hash, size_t offset, size_t size, bool choose_biggest) {
    LOG_DEBUG("chunk_approp_hash_list (hash = [%p], offset = %d, size = %d, choose_biggest = %d)\n",
              hash, offset, size, choose_biggest);

    // if there is no big enough chunk, return NULL
    assert(hash);
    assert(hash_is_ok(hash));
    assert(offset == (size_t)pa_offset(offset));
    assert(size > 0);

    struct hash_list_t *entry_chunk_hash_list = hash_find(hash, offset);
    if (!entry_chunk_hash_list)
        return NULL;

    struct hash_list_t *appropriate_chunk_hash_list = entry_chunk_hash_list;
    struct chunk_t *appropriate_chunk = (chunk_t *)entry_chunk_hash_list->data;

    struct hash_list_t *hash_list_node_iterator = entry_chunk_hash_list->next;

    int sign = (choose_biggest) ? -1 : 1;

    while (hash_list_node_iterator) {
        if (hash_list_node_iterator->cmp_identity != (ssize_t)offset) {
            hash_list_node_iterator = hash_list_node_iterator->next;
            continue;
        }

        struct chunk_t *approp_chunk_temp = (chunk_t *)hash_list_node_iterator->data;
        if (approp_chunk_temp && approp_chunk_temp->mapped_area_size
                                 - (offset - approp_chunk_temp->pa_offset) >= size) {
            assert(!chunk_is_empty(approp_chunk_temp));
            // big enough
            // it is possible few strategies here: first one, the biggest, the smallest
            if (!appropriate_chunk ||
                sign*(appropriate_chunk->mapped_area_size - approp_chunk_temp->mapped_area_size) > 0) {
                // optimal: the least or the biggest
                appropriate_chunk = approp_chunk_temp;
                appropriate_chunk_hash_list = hash_list_node_iterator;
            }
        }

        hash_list_node_iterator = hash_list_node_iterator->next;
    }

    assert(((chunk_t *)appropriate_chunk_hash_list->data) == appropriate_chunk);

    struct hash_list_t *ret_hash_list =  appropriate_chunk
                                         && appropriate_chunk->mapped_area_size
                                            - (offset - appropriate_chunk->pa_offset) >= size
                                         ? appropriate_chunk_hash_list : NULL;

    return ret_hash_list;
}

struct chunk_t *chunk_get(struct mapped_file_t *mapped_file, off_t offset, size_t size, bool choose_biggest) {
    LOG_DEBUG("Called chunk_get(mapped_file = %p, offset = %d, size = %d).\n", mapped_file, offset, size);
    assert(mapped_file);
    assert(mapped_file_is_ok(mapped_file));
    assert(offset == pa_offset(offset));
    assert(offset <= (off_t)mapped_file->file_size);
    assert(size > 0);

    struct hash_list_t *chunk_hash_list = chunk_approp_hash_list(mapped_file->chunk_ptr_ht,
                                                                 (size_t) offset, size,
                                                                 choose_biggest);
    if (chunk_hash_list && chunk_hash_list->data) {
        chunk_t *chunk = (chunk_t *)chunk_hash_list->data;
        assert(chunk->mapped_area);
        assert(chunk->hash_list);
        assert(chunk->mapped_area_size - (offset - chunk->pa_offset) >= (ssize_t)size);

        LOG_DEBUG("chunk_get: appropriate chunk already exists\n", NULL);
        return chunk;
    }

    LOG_DEBUG("chunk_get: looking for free place in pool for chunk placement\n", NULL);
    ssize_t chunk_pool_w_idx_new = chunk_pool_place_idx_find(mapped_file);
    if (chunk_pool_w_idx_new < 0) {
        LOG_ERROR("chunk_get: chunk pull is full; unacquire unclaimed chunks when it is possible.\n", NULL);
        return NULL;
    }

    // placing
    mapped_file->chunk_pool_w_idx = (chunk_pool_w_idx_new + 1) % mapped_file->chunk_pool_size;

    chunk_t *chunk = &(mapped_file->chunk_pool[chunk_pool_w_idx_new]);
    assert(chunk);
    assert(chunk->reference_counter == 0);

    if (chunk->mapped_area) {
        // destruct old reference
        assert(chunk->hash_list);
        hash_delete_node(mapped_file->chunk_ptr_ht, chunk->hash_list);

        ht_node_ref_t *ht_node = chunk->ht_node;
        while (ht_node) {
            hash_delete_node(mapped_file->chunk_ptr_ht, ht_node->hash_list);
            ht_node = ht_node->next;
        }

        chunk->ht_node = NULL;
        if (chunk->in_cache) {
            mapped_file->cache = NULL;
            chunk->in_cache = false;
        }
        chunk_unmap(chunk);
    }

    assert(chunk_is_empty(chunk));
    assert(!chunk_hash_list);

    chunk_hash_list = hash_node_construct(mapped_file->chunk_ptr_ht, (size_t)offset, chunk);
    assert(chunk_hash_list);

    hash_add_node(mapped_file->chunk_ptr_ht, chunk_hash_list);

    chunk->pa_offset = offset;
    chunk->hash_list = chunk_hash_list;

    return chunk;
}

int chunk_map(struct chunk_t *chunk, int fd, off_t offset, size_t size) {
    LOG_DEBUG("Called chunk_map (chunk = [%p], offset = %u, length = %u).\n", chunk, offset, size);
    assert(chunk);
    assert(size);
    assert(offset == pa_offset(offset));
    assert(!chunk->mapped_area);

    chunk->mapped_area = mmap(NULL, size, PROT_WRITE, MAP_SHARED, fd, offset);
    assert(chunk->mapped_area != MAP_FAILED);

    if (chunk->mapped_area == MAP_FAILED) {
        LOG_ERROR("chunk_map: mmap offset [%p] failed: %s.\n", offset, strerror(errno));
        chunk->mapped_area = NULL;
        chunk->mapped_area_size = -1;

#ifdef DEBUG
        chunk->hash_list = NULL;
        chunk->ht_node = NULL;
        chunk->pa_offset = (off_t)(-1);
        chunk->reference_counter = -1;
#endif

        return -1;
    }

    chunk->mapped_area_size = size;
    chunk->in_cache = false;
    return 0;
}

void chunk_unmap(struct chunk_t *chunk) {
    LOG_DEBUG("Called chunk_unmap (chunk = [%p]).\n", chunk);
    assert(chunk);

    if (chunk->reference_counter > 0) {
        LOG_WARN("chunk_unmap: chunk reference_counter is greater than 0\n", NULL);
    }

    if (chunk->mapped_area) {
        munmap(chunk->mapped_area, (size_t)chunk->mapped_area_size);
    }

    chunk->mapped_area = NULL;
    chunk->mapped_area_size = 0;

#ifdef DEBUG
    chunk->hash_list = NULL;
    chunk->ht_node = NULL;
    chunk->pa_offset = (off_t)(-1);
    chunk->reference_counter = -1;
#endif
}
