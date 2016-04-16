//
// Created by kir on 16.04.16.
//

#include "mapped_file.h"
#include "hashtable.h"
#include <fcntl.h>
#include "logger.h"
#include "hash_table.h"
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>


const size_t Chunk_size = 4096;
const size_t Hash_table_size = 32768;
const size_t Chunk_pool_size = 1;

struct chunk_t {
    void *mapped_area;
    size_t mapped_area_size; // % page_size == 0
    off_t offset_key;
    size_t reference_counter;
};

struct mapped_file_t {
    int fd;
    size_t file_size;
    size_t chunk_std_size;
    struct hash_t *chunk_ptr_ht;

    size_t chunk_pool_size;
    struct chunk_t *chunk_pool;
    size_t chunk_pool_w_idx;

    size_t max_memory_usage;
    size_t total_memory_usage;
};

struct mf_mapmem_info_t {
    void *m_area_ptr;
    size_t length;
    chunk_t *chunk;
};

static off_t pa_offset(off_t offset);
size_t hash_func(const size_t cmp_identity);
int chunk_map(struct chunk_t *chunk, int fd, off_t offset, size_t length);
void chunk_unmap(struct chunk_t *chunk);
ssize_t chunk_pool_place_idx_find(struct mapped_file_t *mapped_file);
struct chunk_t *chunk_get_by_offset(struct mapped_file_t *mapped_file, off_t offset);
void *chunk_mem_acquire(const struct mapped_file_t *mapped_file, chunk_t *chunk, off_t offset);
void chunk_mem_unacquire(struct chunk_t *chunk);
ssize_t mapped_file_data_memcpy(struct mapped_file_t *mapped_file, off_t offset, size_t size, void *buf,
                                bool write_mode);


mf_handle_t mf_open(const char *pathname, size_t max_memory_usage) {
    LOG_DEBUG("Called mf_open (pathname = %s, max_memory_usage = %u).\n", pathname, max_memory_usage);
    if (!pathname) {
        LOG_ERROR("mf_open: Function argument pathname is a NULL pointer. Exit.\n", NULL);
        return NULL;
    }

    mf_handle_t mf = calloc(1, sizeof(struct mapped_file_t));
    if (!mf) {
        LOG_ERROR("mf_open: calloc: Failed memory allocation for mapped_file_t structure.\n", NULL);
        return NULL;
    }

    struct mapped_file_t *mapped_file = (struct mapped_file_t *) (mf);

    int fd;
    if ((fd = open(pathname, O_RDWR)) < 0) {
        LOG_ERROR("mf_open: open: failed open file.\n", NULL);
        free(mf);
        return NULL;
    }
    mapped_file->fd = fd;

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        LOG_ERROR("mf_open: fstat: failed obtaining file info using fstat.\n", NULL);
        close(fd);
        free(mf);
        return NULL;
    }
    mapped_file->file_size = (size_t) sb.st_size;

    mapped_file->chunk_std_size = (size_t) pa_offset(Chunk_size);

    mapped_file->chunk_ptr_ht = hash_construct(Hash_table_size, hash_func);
    if (!mapped_file->chunk_ptr_ht) {
        LOG_ERROR("mf_open: hash_construct: failed construct hash table.\n", NULL);
        close(fd);
        free(mf);
        return NULL;
    }

    mapped_file->chunk_pool_size = Chunk_pool_size;
    mapped_file->chunk_pool_w_idx = 0;
    mapped_file->chunk_pool = (struct chunk_t *) calloc(Chunk_pool_size, sizeof(struct chunk_t));
    if (!mapped_file->chunk_pool) {
        LOG_ERROR("\"mf_open: failed allocate momory for chunk_pool\n", NULL);
        hash_destruct(mapped_file->chunk_ptr_ht);
        close(fd);
        free(mf);
        return NULL;
    }


    mapped_file->max_memory_usage = max_memory_usage;
    mapped_file->total_memory_usage = 0;

    LOG_DEBUG("mf_open: mf structure were constructed and inited, return [%p].\n\n", mf);
    return mf;
}

int mf_close(mf_handle_t mf) {
    LOG_DEBUG("Called mf_close(mf = [%p]).\n", mf);
    if (!mf) {
        LOG_ERROR("mf_open: Function argument mf is a NULL pointer. Exit.\n", NULL);
        return -1;
    }

    struct mapped_file_t *mapped_file = (struct mapped_file_t *) (mf);

    for (size_t i = 0; i < mapped_file->chunk_pool_size; i++) {
        chunk_unmap(&(mapped_file->chunk_pool[i]));
    }

    free((void *) mapped_file->chunk_pool);
    hash_destruct(mapped_file->chunk_ptr_ht);
    close(mapped_file->fd);
    free(mf);

    LOG_DEBUG("mf_close: destruction finished.\n\n", NULL);
    return 0;
}

mf_mapmem_t *mf_map(mf_handle_t mf, off_t offset, size_t size) {
    if (!mf || size == 0) {
        LOG_ERROR("mf_map: Invalid function argument. Exit.\n", NULL);
        return NULL;
    }

    mf_mapmem_t *mapmem = (mf_mapmem_t *)malloc(sizeof(mf_mapmem_t));
    if (!mapmem)
        return NULL;
    mapmem->handle = calloc(1, sizeof(mf_mapmem_info_t));
    if (!mapmem->handle)
        return NULL;

    mf_mapmem_info_t *mapmem_info = (mf_mapmem_info_t *)mapmem->handle;

    mapped_file_t *mapped_file = (mapped_file_t *)mf;
    if (size <= mapped_file->chunk_std_size) {
	    chunk_t *chunk = chunk_get_by_offset(mapped_file, offset);
	    if (chunk) {
	        chunk_mem_acquire(mapped_file, chunk, offset);
	        if (chunk->mapped_area_size - (offset - chunk->offset_key) >= size) {
	            mapmem->ptr = (void *)((size_t)chunk->mapped_area + (offset - chunk->offset_key));
                mapmem_info->chunk = chunk;
	            return mapmem;
	        }
	        chunk_mem_unacquire(chunk);
	    }
	}

    size_t length = (offset - pa_offset(offset)) + size;
    mapmem->ptr = mmap(NULL, length, PROT_WRITE, MAP_SHARED, mapped_file->fd, pa_offset(offset));
    if (mapmem->ptr == MAP_FAILED) {
        LOG_ERROR("mf_map: mmap offset [%p] failed: %s.\n", offset, strerror(errno));
        free(mapmem);
        return NULL;
    }
    mapmem_info->m_area_ptr = mapmem->ptr;
    mapmem->ptr = (void *)((size_t)mapmem->ptr + offset - pa_offset(offset));
    mapmem_info->length = length;

    return mapmem;
}

int mf_unmap(mf_mapmem_t *mm) {
    if (!mm) {
        LOG_ERROR("mf_unmap: Invalid function argument mm == NULL. Exit.\n", NULL);
        return -1;
    }

    mf_mapmem_info_t *mapmem_info = (mf_mapmem_info_t *)mm->handle;

    if (mapmem_info->chunk) {
        chunk_mem_unacquire(mapmem_info->chunk);
    } else {
        munmap(mapmem_info->m_area_ptr, mapmem_info->length);
    }

    free((void *)mapmem_info);
    free((void *)mm);

    return 0;
}

ssize_t mf_file_size(mf_handle_t mf) {
    LOG_DEBUG("Called mf_file_size(mf = [%p]).\n", mf);
    if (!mf) {
        LOG_WARN("mf_file_size: Function argument mf is a NULL pointer. Exit.\n", NULL);
        return -1;
    }

    return ((mapped_file_t *)mf)->file_size;
}



ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void *buf) {
    LOG_DEBUG("Called mf_read (mf = [%p], offset = %u, size = %u, buf = [%p]).\n",
              mf, offset, size, buf);
    if (!mf || size == 0 || !buf) {
        LOG_ERROR("mf_read: Invalid function argument. Exit.\n", NULL);
        return -1;
    }

    return mapped_file_data_memcpy((struct mapped_file_t *) mf, offset, size, buf, false);
}

ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void *buf) {
    LOG_DEBUG("Called mf_write (mapped_file = [%p], offset = %u, size = %u, buf = [%p]).\n",
              mf, offset, size, buf);
    if (!mf || size == 0 || !buf) {
        LOG_ERROR("mf_write: Invalid function argument. Exit.\n", NULL);
        return -1;
    }

    return mapped_file_data_memcpy((struct mapped_file_t *) mf, offset, size, (void *) buf, true);
}

ssize_t mapped_file_data_memcpy(struct mapped_file_t *mapped_file, off_t offset, size_t size, void *buf,
                                bool write_mode) {
    assert(mapped_file);
    assert(size != 0);
    assert(buf);

    off_t end_offset = offset + size;
    size_t bytes_processed = 0;
    bool should_exit = false;

    while (offset < end_offset) {
        chunk_t *chunk = chunk_get_by_offset(mapped_file, offset);
        if (!chunk) {
            LOG_ERROR("mapped_file_data_memcpy: Failed chunk_get_by_offset\n", NULL);
            return (size_t)(-1);
        }

        void *data_ptr = chunk_mem_acquire(mapped_file, chunk, offset);
        if (!data_ptr) {
            LOG_ERROR("mapped_file_data_memcpy: failed chunk_mem_acquire\n", NULL);
            return (size_t)(-1);
        }

        //TODO: write out of file???
        size_t bytes_to_chunk_end = chunk->mapped_area_size - ((off_t)data_ptr - pa_offset((off_t)data_ptr));

        size_t bytes_to_process = size - bytes_processed;

        if (bytes_to_process <= bytes_to_chunk_end) {
            should_exit = true;
        } else {
            bytes_to_process = bytes_to_chunk_end;
        }

        if (write_mode) {
            memcpy(data_ptr, (void *)((size_t)buf + bytes_processed), bytes_to_process);
        } else {
            memcpy((void *)((size_t)buf + bytes_processed), data_ptr, bytes_to_process);
        }

        bytes_processed += bytes_to_process;

        chunk_mem_unacquire(chunk);

        offset += bytes_to_chunk_end;
        if (offset >= (off_t)mapped_file->file_size || should_exit)
            break;

        offset = pa_offset(offset);
    }

    return bytes_processed == 0 ? (size_t)(-1) : bytes_processed;
}


void *chunk_mem_acquire(const struct mapped_file_t *mapped_file, chunk_t *chunk, off_t offset) {
    LOG_DEBUG("Called chunk_mem_acquire (mf = [%p], chunk = [%p], offset = %u).\n",
              mapped_file, chunk, offset);
    assert(mapped_file);
    assert(chunk);

    off_t chunk_pa_offset = pa_offset(offset);

    if (!chunk->mapped_area) {
        LOG_DEBUG("chunk_mem_acquire: opening the gate in chunk.\n", NULL);

        chunk->reference_counter = 0;
        size_t length = mapped_file->chunk_std_size;

        if (chunk_pa_offset + length > mapped_file->file_size) {
            length = mapped_file->file_size - chunk_pa_offset;
        }

        if (chunk_map(chunk, mapped_file->fd, chunk_pa_offset, length) != 0) {
            LOG_ERROR("chunk_mem_acquire: failed chunk_map\n", NULL);
            return NULL;
        }
    } else {
        LOG_DEBUG("chunk_mem_acquire: chunk already has a gate.\n", NULL);
    }

    chunk->reference_counter++;
    return (void *) ((size_t) chunk->mapped_area + (offset - chunk_pa_offset));
}

void chunk_mem_unacquire(struct chunk_t *chunk) {
    LOG_DEBUG("Called chunk_mem_unacquire (chunk = [%p]).\n", chunk);
    assert(chunk);

    chunk->reference_counter--;
}

ssize_t chunk_pool_place_idx_find(struct mapped_file_t *mapped_file) {
    assert(mapped_file);

    size_t chunk_pool_size = mapped_file->chunk_pool_size;
    size_t chunk_pool_w_idx = mapped_file->chunk_pool_w_idx;
    chunk_t *chunk_pool = mapped_file->chunk_pool;

    ssize_t chunk_pool_w_idx_new = -1;

    for (size_t w_idx = chunk_pool_w_idx; w_idx < chunk_pool_size + chunk_pool_w_idx; w_idx++) {
        size_t chunk_pool_elem_idx = w_idx % chunk_pool_size;
        chunk_t *chunk = &(chunk_pool[chunk_pool_elem_idx]);

        if (!chunk->mapped_area || chunk->reference_counter == 0) {
            chunk_pool_w_idx_new = chunk_pool_elem_idx;
            break;
        }
    }

    return chunk_pool_w_idx_new;
}

struct chunk_t *chunk_get_by_offset(struct mapped_file_t *mapped_file, off_t offset) {
    LOG_DEBUG("Called chunk_get_by_offset(mapped_file = %p, offset = %d).\n", mapped_file, offset);
    assert(mapped_file);

    if (offset >= (off_t) mapped_file->file_size) {
        LOG_WARN("chunk_get_by_offset: offset is out of the file.\n", NULL);
        return NULL;
    }

    size_t chunk_pa_offset = (size_t) pa_offset(offset);

    struct hash_list_t *chunk_hash_list_node = hash_add(mapped_file->chunk_ptr_ht, chunk_pa_offset, NULL);
    if (!chunk_hash_list_node) {
        LOG_ERROR("chunk_get_by_offset: failed hash_add!.\n", NULL);
        return NULL;
    }

    chunk_t *chunk = (struct chunk_t *) chunk_hash_list_node->data;
    if (chunk) {
        LOG_DEBUG("chunk_get_by_offset: chunk already exists.\n", NULL);
        return chunk;
    }

    LOG_DEBUG("chunk_get_by_offset: looking for free place for chunk placement\n", NULL);
    ssize_t chunk_pool_w_idx_new = chunk_pool_place_idx_find(mapped_file);
    if (chunk_pool_w_idx_new < 0) {
        LOG_ERROR("chunk_get_by_offset: chunk pull is full; unacquire unclaimed chunks when it is possible.\n",
                  NULL);
        return NULL;
    }

    // placing
    mapped_file->chunk_pool_w_idx = (chunk_pool_w_idx_new + 1) % mapped_file->chunk_pool_size;

    chunk = &(mapped_file->chunk_pool[chunk_pool_w_idx_new]);
    if (chunk->mapped_area) {
        // destruct old reference
        //TODO: delete elem from hashtable
        hash_find(mapped_file->chunk_ptr_ht, (size_t)chunk->offset_key)->data = NULL;
        chunk_unmap(chunk);
    }
    chunk_hash_list_node->data = (void *)chunk;

    chunk->offset_key = chunk_pa_offset;

    return chunk;
}

int chunk_map(struct chunk_t *chunk, int fd, off_t offset, size_t length) {
    LOG_DEBUG("Called chunk_map (chunk = [%p], offset = %u, length = %u).\n", chunk, offset, length);
    assert(chunk);

    if (chunk->mapped_area) {
        LOG_WARN("chunk_map: chunk already has connected memory!\n", NULL);
    }

    assert(offset == pa_offset(offset));

    chunk->mapped_area = mmap(NULL, length, PROT_WRITE, MAP_SHARED, fd, offset);
    chunk->mapped_area_size = length;

    if (chunk->mapped_area == MAP_FAILED) {
        LOG_ERROR("chunk_map: mmap offset [%p] failed: %s.\n", offset, strerror(errno));
        chunk->mapped_area = NULL;
        chunk->mapped_area_size = 0;
        return -1;
    }

    return 0;
}

void chunk_unmap(struct chunk_t *chunk) {
    LOG_DEBUG("Called chunk_unmap (chunk = [%p]).\n", chunk);
    assert(chunk);

    if (chunk->reference_counter > 0) {
        LOG_WARN("chunk_unmap: chunk reference_counter is greater than 0\n", NULL);
    }

    if (chunk->mapped_area) {
        munmap(chunk->mapped_area, chunk->mapped_area_size);
    }

    chunk->mapped_area = NULL;
    chunk->mapped_area_size = 0;
}


static off_t pa_offset(off_t offset) {
    return (off_t) (offset & ~(sysconf(_SC_PAGE_SIZE) - 1));
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