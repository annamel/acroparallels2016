//
// Created by kir on 16.04.16.
//

#include "logger.h"
#include "mapped_file.h"
#include "chunk_manager.h"

const size_t Chunk_size = 4096;
const size_t Hash_table_size = 32768;
const size_t Chunk_pool_size = 4;

mf_handle_t mf_open(const char *pathname) {
    LOG_DEBUG("Called mf_open (%s)\n", pathname);
    if (!pathname) {
        LOG_ERROR("mf_open: Function argument pathname is a NULL pointer. Exit.\n", NULL);
        return NULL;
    }

    mf_handle_t mf = (mf_handle_t)mapped_file_construct(pathname, Chunk_size, Hash_table_size, Chunk_pool_size);

    LOG_DEBUG("mf_open: mf structure were constructed and inited, return [%p].\n\n", mf);
    return mf;
}

int mf_close(mf_handle_t mf) {
    LOG_DEBUG("Called mf_close(mf = [%p]).\n", mf);
    if (!mf) {
        LOG_ERROR("mf_open: Function argument mf is a NULL pointer. Exit.\n", NULL);
        return -1;
    }

    mapped_file_destruct((mapped_file_t *)mf);

    LOG_DEBUG("mf_close: destruction finished.\n\n", NULL);
    return 0;
}

off_t mf_file_size(mf_handle_t mf) {
    LOG_DEBUG("Called mf_file_size(mf = [%p]).\n", mf);
    if (!mf) {
        LOG_WARN("mf_file_size: Function argument mf is a NULL pointer. Exit.\n", NULL);
        return -1;
    }

    return ((mapped_file_t *)mf)->file_size;
}


ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset) {
    LOG_DEBUG("Called mf_read (mf = [%p], offset = %u, size = %u, buf = [%p]).\n",
              mf, offset, count, buf);
    if (!mf || count == 0 || !buf) {
        LOG_ERROR("mf_read: Invalid function argument. Exit.\n", NULL);
        return -1;
    }

    return mapped_file_data_memcpy((struct mapped_file_t *) mf, offset, count, buf, false);
}

ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset) {
    LOG_DEBUG("Called mf_write (mf = [%p], offset = %u, size = %u, buf = [%p]).\n",
              mf, offset, count, buf);
    if (!mf || count == 0 || !buf) {
        LOG_ERROR("mf_write: Invalid function argument. Exit.\n", NULL);
        return -1;
    }

    return mapped_file_data_memcpy((struct mapped_file_t *) mf, offset, count, (void *) buf, true);
}

void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle) {
    LOG_DEBUG("Called mf_map (mf = [%p], offset = %u, size = %u, mapmem_handle = [%p])\n",
              mf, offset, size, mapmem_handle);
    if (!mf || size <= 0 || !mapmem_handle) {
        LOG_ERROR("mf_map: Invalid function argument. Exit.\n", NULL);
        return NULL;
    }

    size_t read_size;
    return chunk_mem_acquire((mapped_file_t *)mf, offset, size, false, &read_size, (chunk_t **)mapmem_handle);
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle) {
    LOG_DEBUG("Called mf_unmap (mf = [%p], mapmem_handle = [%p])\n",
              mf, mapmem_handle);

    if (!mf || !mapmem_handle) {
        LOG_ERROR("mf_write: Invalid function argument. Exit.\n", NULL);
        return -1;
    }

    chunk_mem_unacquire((chunk_t *)mapmem_handle);
    return 0;
}

