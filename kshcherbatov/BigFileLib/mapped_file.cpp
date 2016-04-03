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
    size_t file_size;
    size_t chunk_std_size;
    struct hash_t *chunk_pool_ht;
};

int mf_open(const char *name, size_t pool_size, size_t chunk_size, int read_only, mf_handle_t *mf);
int mf_close(mf_handle_t *mf);
int mf_chunk_acquire(struct mapped_file_t *mapped_file, chunk_t *chunk, size_t offset, void **ptr);
size_t hash_func(const size_t cmp_identity);
int hash_cmp_func(const size_t cmp_identity_a, const size_t cmp_identity_b);
void chunk_destruct(void *data);
static size_t pa_offset(size_t offset);
struct chunk_t *get_chunk_by_offset(const struct mapped_file_t *mapped_file, size_t offset);

int mf_open(const char *name, size_t pool_size, size_t chunk_size, int read_only, mf_handle_t *mf) {
    //TODO: error codes according specification
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

    //TODO: readonly?
    int fd;
    if ((fd = open(name, (read_only ? O_RDONLY : O_RDWR))) < 0) {
        LOG_ERROR("mf_open: failed open file.\n", NULL);
        *mf = NULL;
        return ENOENT;
    }
    mapped_file->fd = fd;

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        LOG_ERROR("mf_open: failed obtaining file info using fstat.\n", NULL);
        *mf = NULL;
        return EXIT_FAILURE;
    }
    mapped_file->file_size = (size_t)sb.st_size;

    mapped_file->chunk_std_size = (!chunk_size) ? (size_t)sysconf(_SC_PAGE_SIZE) : pa_offset(chunk_size);
    LOG_DEBUG("mf_open: chunk_std_size is %d.\n", mapped_file->chunk_std_size);

    mapped_file->chunk_pool_ht = hash_construct(pool_size,
                                                hash_func, hash_cmp_func, chunk_destruct,
                                                sizeof(struct chunk_t));

    if (!mapped_file->chunk_pool_ht) {
        LOG_ERROR("mf_open: failed construct hash table.\n", NULL);
        close(fd);
        free((void*)mapped_file);
        *mf = NULL;
        return ENOMEM;
    }

    LOG_DEBUG("mf_open: mf structure were constructed and inited, return [%p].\n\n", mf);
    return EXIT_SUCCESS;
}

int mf_read(mf_handle_t *mf, size_t start, size_t length) {
    LOG_DEBUG("Called mf_read (mf = [%p], offset = %u, length = %u).\n\n",
              mf, start, length);

    if (!mf) {
        //TODO: Log MSG
        return EINVAL;
    }

    struct mapped_file_t *mapped_file = (struct mapped_file_t *)(*mf);

    //TODO: fucking loop!
    size_t end = start + length;
    while (start < end) {
        void *data_ptr;

        chunk_t *chunk = get_chunk_by_offset(mapped_file, start);
        if (!chunk)
            break;

        if (mf_chunk_acquire(mapped_file, chunk, start, &data_ptr) != 0) {
            LOG_DEBUG("mf_read: failed mf_chunk_acquire\n", NULL);
            break;
        }

        write(STDOUT_FILENO, data_ptr, chunk->mapped_area_size -
                ((size_t)data_ptr - pa_offset((size_t)data_ptr)));

        size_t step = chunk->mapped_area_size;
        mf_chunk_unacquire(mf, start);

        start += step;
        if (start >= mapped_file->file_size)
            break;

        start = pa_offset(start);
    }

    LOG_DEBUG("mf_read: finish.\n\n", NULL);
    return EXIT_SUCCESS;
}

int mf_chunk_acquire(struct mapped_file_t *mapped_file, chunk_t *chunk, size_t offset, void **ptr) {
    LOG_DEBUG("Called mf_chunk_acquire (mf = [%p], chunk = [%p], offset = %u, ptr = [%p]).\n",
              mapped_file, chunk, offset, ptr);
    assert(mapped_file);
    assert(chunk);
    assert(ptr);

    size_t chunk_pa_offset = pa_offset(offset);

    // every correct chunk has a nonzero pointer to memory stored in data field
    if (!chunk->mapped_area) {
        LOG_DEBUG("mf_chunk_acquire: opening the gate in chunk.\n", NULL);

        chunk->reference_counter = 0;

        size_t length = mapped_file->chunk_std_size;

        if (chunk_pa_offset + length > mapped_file->file_size) {
            LOG_DEBUG("JUMP OUT OF FILE!\n", NULL);
            length = mapped_file->file_size - chunk_pa_offset;
        }

        LOG_DEBUG("mf_chunk_acquire: Call mmap(NULL, length = %d, fd = %d, offset = %p)\n",
                  (unsigned)length, mapped_file->fd, chunk_pa_offset);

        chunk->mapped_area = mmap(NULL, length, PROT_WRITE, MAP_SHARED, mapped_file->fd, chunk_pa_offset);

        if (chunk->mapped_area == MAP_FAILED) {
            LOG_ERROR("mf_chunk_acquire: mmap offset [%p] failed: %s.\n", chunk_pa_offset, strerror(errno));
            chunk->mapped_area = NULL;
            chunk->mapped_area_size = 0;
            *ptr = NULL;
            return errno;
        }

        chunk->mapped_area_size = length;
    } else {
        LOG_DEBUG("mf_chunk_acquire: chunk already have had a gate.\n", NULL);
    }

    chunk->reference_counter++;
    (*ptr) = (void *)((size_t)chunk->mapped_area + (offset - chunk_pa_offset));

    LOG_DEBUG("mf_chunk_acquire: return [%p].\n\n", *ptr);

    return EXIT_SUCCESS;
}

int mf_chunk_unacquire(mf_handle_t *mf, size_t offset) {
    LOG_DEBUG("Called mf_chunk_unacquire (mf = [%p], offset = %u).\n",
              mf, offset);

    if (!mf || !(*mf)) {
        LOG_WARN("mf_chunk_unacquire: wrong argument!.\n", NULL);
        return EINVAL;
    }

    struct mapped_file_t *mapped_file = (struct mapped_file_t *)(*mf);

    chunk_t *chunk = get_chunk_by_offset(mapped_file, offset);
    assert(chunk);

    if (--chunk->reference_counter <= 0) {
        LOG_DEBUG("mf_chunk_unacquire: chunk is unused. destruct it\n", NULL);
        chunk_destruct((void *)chunk);
        chunk->mapped_area = NULL;
    }

    LOG_DEBUG("mf_chunk_unacquire: exit\n\n", NULL);

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

    hash_destruct(mapped_file->chunk_pool_ht);
    close(mapped_file->fd);
    free((void *)mapped_file);

    LOG_DEBUG("mf_close: destruction finished.\n\n", NULL);
    return EXIT_SUCCESS;
}


size_t hash_func(const size_t cmp_identity) {
    LOG_DEBUG("Called hash_func(cmp_identity = %u).\n", cmp_identity);

    uint64_t key = (uint64_t)cmp_identity;
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccd;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53;
    key ^= key >> 33;

    LOG_DEBUG("hash_func: return %u.\n", key);
    return key;
}

int hash_cmp_func(const size_t cmp_identity_a, const size_t cmp_identity_b) {
    LOG_DEBUG("Called hash_cmp_func(cmp_identity_a = %u, cmp_identity_b = %u).\n", cmp_identity_a, cmp_identity_b);

    if (cmp_identity_a == cmp_identity_b)
        return 0;
    else
        return 1;
}


void chunk_destruct(void *data) {
    LOG_DEBUG("Called chunk_destruct(data = %p).\n", data);
    assert(data);

    chunk_t *chunk = (struct chunk_t *)data;
    if (!chunk->mapped_area)
        return;

    if (chunk->reference_counter > 0) {
       LOG_WARN("chunk_destruct: reference counter of destructing chunk is greater then 0.\n", NULL);
    }

    if (munmap(chunk->mapped_area, chunk->mapped_area_size)) {
        LOG_WARN("chunk_destruct: failed munmap memory.\n", NULL);
    }
}

struct chunk_t *get_chunk_by_offset(const struct mapped_file_t *mapped_file, size_t offset) {
    LOG_DEBUG("Called get_chunk_hash_node_by_offset(mapped_file = %p, offset = %d).\n", mapped_file, offset);
    assert(mapped_file);

    if (offset >= mapped_file->file_size) {
        LOG_ERROR("get_chunk_by_off: offset is out of the file.\n", NULL);
        return NULL;
    }

    size_t chunk_pa_offset = pa_offset(offset);

    // create empty node if needed
    struct hash_list_t *chunk_hash_list_node = hash_add(mapped_file->chunk_pool_ht, chunk_pa_offset, NULL);

    if (!chunk_hash_list_node) {
        LOG_ERROR("get_chunk_by_off: failed hash_add!.\n", NULL);
        return NULL;
    }

    return (chunk_t *)chunk_hash_list_node->data;
}

static size_t pa_offset(size_t offset) {
    return (size_t)(offset & ~(sysconf(_SC_PAGE_SIZE) - 1));
}