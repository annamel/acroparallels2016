#include "mapped_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>



#include "chunk_manager/chunk_manager.h"
#include "logger/logger.h"



static int mf_get_ial(off_t offset, size_t size, off_t* index, off_t* len)
{
    off_t pa_offset = offset & ~(sysconf(_SC_PAGESIZE) - 1);
    off_t new_size = size + offset - pa_offset;

    *index = pa_offset / get_chunk_size(1);
    *len = new_size / get_chunk_size(1);
    if(new_size % get_chunk_size(1)) (*len)++;

    return 0;
}



mf_handle_t mf_open(const char *pathname)
{
    log_write(DEBUG, "mf_open: started");

    if(!pathname)
    {
        log_write(DEBUG, "mf_open: bad filename");
        return MF_OPEN_FAILED;
    }

    log_write(DEBUG, "mf_open: started");

    int fd = open(pathname, O_RDWR, 0666);
    if(fd == -1)
    {
        log_write(ERROR, "mf_open: can't open file:%s, errno=%d",
                  pathname, errno);
        return MF_OPEN_FAILED;
    }

    chpool_t *chpool = chp_init(fd, PROT_READ | PROT_WRITE);
    if(!chpool)
    {
        log_write(ERROR, "mf_open: can't init chunk pool");
        return MF_OPEN_FAILED;
    }

    log_write(DEBUG, "mf_open: started");
    return (mf_handle_t)chpool;
}



ssize_t mf_read(mf_handle_t mf, void *buf, size_t count, off_t offset)
{
    off_t file_size = mf_file_size(mf);
    if(file_size == -1)
    {
        log_write(ERROR, "mf_read(count=%d, offset=%d): can't get file size",
                  count, offset);
        return -1;
    }

    if(mf == MF_OPEN_FAILED || !buf || offset < 0 || offset > file_size)
    {
        errno = EINVAL;
        log_write(ERROR, "mf_read(count=%d, offset=%d): bad arguments",
                  count, offset);
        return -1;
    }

    log_write(DEBUG, "mf_read(count=%d, offset=%d): started",
              count, offset);

    off_t index;
    off_t len;
    mf_get_ial(offset, count, &index, &len);

    chunk_t *read_chunk;
    int error = chp_find((chpool_t *)mf, index, len, &read_chunk);
    if(error == ENOKEY)
    {
        read_chunk = ch_init(index, len, (chpool_t *)mf);
        if(!read_chunk)
        {
            log_write(ERROR, "mf_read(count=%d, offset=%d): can't init new chunk for read",
                      count, offset);
            return -1;
        }
    }
    else if(error)
    {
        log_write(ERROR, "mf_read(count=%d, offset=%d): chp_find returns error=%d",
                  count, offset, error);
        return -1;
    }

    const void *src = (const void *)(((off_t)(read_chunk->data))
                    + (offset - read_chunk->index * get_chunk_size(1)));
    buf = memcpy(buf, src, count);

    return count;
}



ssize_t mf_write(mf_handle_t mf, const void *buf, size_t count, off_t offset)
{
    off_t file_size = mf_file_size(mf);
    if(file_size == -1)
    {
        log_write(ERROR, "mf_write(count=%d, offset=%d): can't get file size",
                  count, offset);
        return -1;
    }

    if(mf == MF_OPEN_FAILED || !buf || offset < 0 || offset > file_size)
    {
        errno = EINVAL;
        log_write(ERROR, "mf_write(count=%d, offset=%d): bad arguments",
                  count, offset);
        return -1;
    }

    log_write(DEBUG, "mf_write(count=%d, offset=%d): started",
              count, offset);

    off_t index;
    off_t len;
    mf_get_ial(offset, count, &index, &len);

    chunk_t *write_chunk;
    int error = chp_find((chpool_t *)mf, index, len, &write_chunk);
    if(error == ENOKEY)
    {
        write_chunk = ch_init(index, len, (chpool_t *)mf);
        if(!write_chunk)
        {
            log_write(ERROR, "mf_write(count=%d, offset=%d): can't init new chunk for read",
                      count, offset);
            return -1;
        }
    }
    else if(error)
    {
        log_write(ERROR, "mf_write(count=%d, offset=%d): chp_find returns error=%d",
                  count, offset, error);
        return -1;
    }

    void *dst = (void *)(((off_t)(write_chunk->data))
                + (offset - write_chunk->index * get_chunk_size(1)));
    dst = memcpy(dst, buf, count);

    return count;
}



void *mf_map(mf_handle_t mf, off_t offset, size_t size,
             mf_mapmem_handle_t *mapmem_handle)
{
    log_write(DEBUG, "mf_map(offset=%d, size=%d): started",
              offset, size);

    if(mf == MF_OPEN_FAILED || offset < 0 ||
            offset + size > mf_file_size(mf))
    {
        errno = EINVAL;
        log_write(ERROR, "mf_map(offset=%d, size=%d): bad input",
                  offset, size);
        return NULL;
    }

    if(!size)
        return NULL;

    off_t index;
    off_t len;
    mf_get_ial(offset, size, &index, &len);

    int error = chp_find((chpool_t *)mf, index, len, (chunk_t **)mapmem_handle);
    if(error == ENOKEY)
    {
        *mapmem_handle = (mf_mapmem_handle_t *)ch_init(index, len, (chpool_t *)mf);
        if(!(*mapmem_handle))
        {
            log_write(ERROR, "mf_map(offset=%d, size=%d): can't init new chunk",
                      offset, size);
            return NULL;
        }
        ((chunk_t *)(mapmem_handle))->rc ++;

        log_write(DEBUG, "mf_map(offset=%d, size=%d): finished", offset, size);
        return (void *)((off_t)(((chunk_t *)(*mapmem_handle))->data)
                        + (offset - index * get_chunk_size(1)));
    }
    else if(error)
    {
        log_write(ERROR, "mf_map(offset=%d, size=%d): chp_find returns error=%d",
                  offset, size, error);
        return NULL;
    }

    ((chunk_t *)(mapmem_handle))->rc ++;
    log_write(DEBUG, "mf_map(offset=%d, size=%d): finished", offset, size);
    return (void *)((off_t)(((chunk_t *)(*mapmem_handle))->data) +
         (offset - (off_t)((((chunk_t *)(*mapmem_handle))->index)
                                      * get_chunk_size(1))));
}



int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle)
{
    if(mf == MF_OPEN_FAILED || !mapmem_handle)
        return EINVAL;

    log_write(DEBUG, "mf_unmap: started");

    int error = chp_chunk_release((chunk_t *)mapmem_handle);
    if(error)
        return error;

    log_write(DEBUG, "mf_unmap: finished");

    return 0;
}



off_t mf_file_size(mf_handle_t mf)
{
    log_write(DEBUG,"mf_file_size: started");

    if(mf == MF_OPEN_FAILED)
    {
        log_write(ERROR, "mf_file_size: bad mf");
        return EINVAL;
    }

    int fd = ((chpool_t *)mf)->fd;
    if(fd < 0)
    {
        log_write(ERROR, "mf_file_size: bad file descriptor");
        return -1;
    }

    struct stat sb = {0};
    int error = fstat(fd, &sb);
    if(error)
    {
        log_write(ERROR, "mf_file_size: fd returns error=%d", error);
        return -1;
    }

    log_write(DEBUG,"mf_file_size: finished");
    return sb.st_size;
}
