//
// Created by kir on 26.03.16.
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

struct chunk_t {
    // offset is used as a input to make a key in hash table
    void *mapped_area;
    size_t mapped_area_size; // % page_size == 0
    unsigned reference_counter;
};

struct mapped_file_t {
    int fd;
    bool fd_read_only;
    size_t file_size;
    size_t chunk_std_size;
    struct hash_t *chunk_pool_ht;
    size_t chunk_pool_size;
    struct chunk_t *chunk_pool;
};

int mf_open(const char *name, size_t pool_size, size_t chunk_size, int read_only, mf_handle_t *mf);
int mf_close(mf_handle_t *mf);
struct chunk_t *chunk_get_by_offset(const struct mapped_file_t *mapped_file, size_t offset);
void *chunk_mem_acquire(struct mapped_file_t *mapped_file, chunk_t *chunk, size_t offset);
void chunk_mem_unacquire(chunk_t *chunk);
int chunk_map(chunk_t *chunk, int fd, size_t offset, size_t length, bool read_only);
void chunk_unmap(chunk_t *chunk);
size_t hash_func(const size_t cmp_identity);
int hash_cmp_func(const size_t cmp_identity_a, const size_t cmp_identity_b);
void hash_data_destruct_func(void *data);
static size_t pa_offset(size_t offset);

//TODO: error codes according specification

int mf_open(const char *name, size_t pool_size, size_t chunk_size, int read_only, mf_handle_t *mf) {
    LOG_DEBUG("Called mf_open (name = %s, pool_size = %u, chunk_std_size = %u, read_only = %d, mf = [%p]).\n",
             name, pool_size, chunk_size, read_only, mf);

    if (!name || !mf || pool_size == 0) {
        LOG_WARN("mf_open wrong argument!.\n", NULL);
        return EINVAL;
    }

    *mf = malloc(sizeof(struct mapped_file_t));
    if (!(*mf)) {
        LOG_ERROR("mf_open: Failed memory allocation for mapped_file_t structure.\n", NULL);
        return ENOMEM;
    }

    struct mapped_file_t *mapped_file = (struct mapped_file_t *)(*mf);

    int fd;
    if ((fd = open(name, (read_only ? O_RDONLY : O_RDWR))) < 0) {
        LOG_ERROR("mf_open: failed open file.\n", NULL);
        *mf = NULL;
        return ENOENT;
    }
    mapped_file->fd = fd;
    mapped_file->fd_read_only = (bool)read_only;

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        LOG_ERROR("mf_open: failed obtaining file info using fstat.\n", NULL);
        *mf = NULL;
        return EXIT_FAILURE;
    }
    mapped_file->file_size = (size_t)sb.st_size;

    mapped_file->chunk_std_size = (!chunk_size) ? (size_t)sysconf(_SC_PAGE_SIZE) : pa_offset(chunk_size);
    LOG_DEBUG("mf_open: chunk_std_size is %d.\n", mapped_file->chunk_std_size);

    mapped_file->chunk_pool_ht = hash_construct(pool_size, hash_func, hash_cmp_func, hash_data_destruct_func,
                                                sizeof(struct chunk_t));
    if (!mapped_file->chunk_pool_ht) {
        LOG_ERROR("mf_open: failed construct hash table.\n", NULL);
        close(fd);
        free((void*)mapped_file);
        *mf = NULL;
        return ENOMEM;
    }

    mapped_file->chunk_pool_size = pool_size;
    mapped_file->chunk_pool = (struct chunk_t *)calloc(pool_size, sizeof(chunk_t));
    if (!mapped_file->chunk_pool) {
        LOG_ERROR("\"mf_open: failed allocate momory for chunk_pool\n", NULL);
        hash_destruct(mapped_file->chunk_pool_ht);
        close(fd);
        free((void *)mapped_file);
        *mf = NULL;
        return ENOMEM;
    }

    LOG_DEBUG("mf_open: mf structure were constructed and inited, return [%p].\n\n", mf);
    return EXIT_SUCCESS;
}

int mf_close(mf_handle_t *mf) {
    LOG_DEBUG("Called mf_close(mf = [%p]).\n", mf);
    if (!mf) {
        LOG_WARN("mf_close: wrong value.\n", NULL);
        return EINVAL;
    }

    struct mapped_file_t *mapped_file = (struct mapped_file_t *)(*mf);
    if (!mapped_file) {
        LOG_ERROR("Unexpected mf == NULL!.\n", NULL);
        return EINVAL;
    }

    free((void *)mapped_file->chunk_pool);
    hash_destruct(mapped_file->chunk_pool_ht);
    close(mapped_file->fd);
    free((void *)mapped_file);

    LOG_DEBUG("mf_close: destruction finished.\n\n", NULL);
    return EXIT_SUCCESS;
}

int mf_read(mf_handle_t *mf, size_t start, size_t length) {
    LOG_DEBUG("Called mf_read (mf = [%p], offset = %u, length = %u).\n\n",
              mf, start, length);

    assert(mf);

    struct mapped_file_t *mapped_file = (struct mapped_file_t *)(*mf);

    size_t end = start + length;
    while (start < end) {
        chunk_t *chunk = chunk_get_by_offset(mapped_file, start);
        if (!chunk)
            break;

        void *data_ptr = chunk_mem_acquire(mapped_file, chunk, start);
        if (!data_ptr) {
            LOG_DEBUG("mf_read: failed chunk_mem_acquire\n", NULL);
            break;
        }

        write(STDOUT_FILENO, data_ptr, chunk->mapped_area_size -
                ((size_t)data_ptr - pa_offset((size_t)data_ptr)));

        size_t step = chunk->mapped_area_size;
        chunk_mem_unacquire(chunk);

        start += step;
        if (start >= mapped_file->file_size)
            break;

        start = pa_offset(start);
    }

    LOG_DEBUG("mf_read: finish.\n\n", NULL);
    return EXIT_SUCCESS;
}

struct chunk_t *chunk_get_by_offset(const struct mapped_file_t *mapped_file, size_t offset) {
    LOG_DEBUG("Called chunk_get_by_offset(mapped_file = %p, offset = %d).\n", mapped_file, offset);
    assert(mapped_file);

    if (offset >= mapped_file->file_size) {
        LOG_WARN("chunk_get_by_offset: offset is out of the file.\n", NULL);
        return NULL;
    }

    size_t chunk_pa_offset = pa_offset(offset);

    // create empty node if needed
    struct hash_list_t *chunk_hash_list_node = hash_add(mapped_file->chunk_pool_ht, chunk_pa_offset, NULL);
    if (!chunk_hash_list_node) {
        LOG_ERROR("chunk_get_by_offset: failed hash_add!.\n", NULL);
        return NULL;
    }

    return (chunk_t *)chunk_hash_list_node->data;
}

void *chunk_mem_acquire(struct mapped_file_t *mapped_file, chunk_t *chunk, size_t offset) {
    LOG_DEBUG("Called chunk_mem_acquire (mf = [%p], chunk = [%p], offset = %u).\n",
              mapped_file, chunk, offset);

    assert(mapped_file);
    assert(chunk);

    size_t chunk_pa_offset = pa_offset(offset);

    if (!chunk->mapped_area) {
        LOG_DEBUG("chunk_mem_acquire: opening the gate in chunk.\n", NULL);

        chunk->reference_counter = 0;

        size_t length = mapped_file->chunk_std_size;

        if (chunk_pa_offset + length > mapped_file->file_size) {
            length = mapped_file->file_size - chunk_pa_offset;
        }

        if (chunk_map(chunk, mapped_file->fd, chunk_pa_offset, length, mapped_file->fd_read_only) != EXIT_SUCCESS) {
            LOG_ERROR("chunk_mem_acquire: failed chunk_map\n", NULL);
        }

        chunk->mapped_area_size = length;
    } else {
        LOG_DEBUG("chunk_mem_acquire: chunk already have had a gate.\n", NULL);
    }

    chunk->reference_counter++;
    return  (void *)((size_t)chunk->mapped_area + (offset - chunk_pa_offset));
}

void chunk_mem_unacquire(chunk_t *chunk) {
    LOG_DEBUG("Called chunk_mem_unacquire (chunk = [%p]).\n", chunk);

    chunk->reference_counter--;
}

int chunk_map(chunk_t *chunk, int fd, size_t offset, size_t length, bool read_only) {
    LOG_DEBUG("Called chunk_map (chunk = [%p], offset = %u, length = %u).\n", chunk, offset, length);
    assert(chunk);

    if (chunk->mapped_area) {
        LOG_WARN("chunk_map: chunk already has connected memory!\n", NULL);
    }

    int protect_flags = read_only ? PROT_READ : PROT_WRITE;
    chunk->mapped_area = mmap(NULL, length, protect_flags, MAP_SHARED, fd, offset);

    if (chunk->mapped_area == MAP_FAILED) {
        LOG_ERROR("chunk_map: mmap offset [%p] failed: %s.\n", offset, strerror(errno));
        chunk->mapped_area = NULL;
        chunk->mapped_area_size = 0;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void chunk_unmap(chunk_t *chunk) {
    LOG_DEBUG("Called chunk_unmap (chunk = [%p]).\n", chunk);

    assert(chunk);

    if (chunk->reference_counter > 0) {
        LOG_WARN("chunk_unmap: chunk reference_counter is greater than 0\n", NULL);
    }

    if (munmap(chunk->mapped_area, chunk->mapped_area_size)) {
        LOG_WARN("chunk_unmap: failed munmap memory.\n", NULL);
    }

    chunk->mapped_area = NULL;
    chunk->mapped_area_size = 0;
}

size_t hash_func(const size_t cmp_identity) {
    uint64_t key = (uint64_t)cmp_identity;

    key ^= key >> 33;
    key *= 0xff51afd7ed558ccd;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53;
    key ^= key >> 33;

    return key;
}

int hash_cmp_func(const size_t cmp_identity_a, const size_t cmp_identity_b) {
    if (cmp_identity_a == cmp_identity_b)
        return 0;
    else
        return 1;
}

void hash_data_destruct_func(void *data) {
    LOG_DEBUG("Called hash_data_destruct_func(data = %p).\n", data);
    assert(data);

    chunk_t *chunk = (struct chunk_t *)data;
    if (!chunk->mapped_area)
        return;

    chunk_unmap(chunk);
}

static size_t pa_offset(size_t offset) {
    return (size_t)(offset & ~(sysconf(_SC_PAGE_SIZE) - 1));
}