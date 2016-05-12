#include "mapped_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>



#include "../chunk_manager/chunk_manager.h"
#include "../logger/logger.h"



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
    log_write(INFO, "mf_open: started");
    if(!pathname)
    {
        log_write(ERROR, "mf_open: bad filename");
        errno = EINVAL;
        return MF_OPEN_FAILED;
    }


    int fd = open(pathname, O_RDWR, 0666);
    if(fd == -1)
    {
        log_write(ERROR, "mf_open: can't open file, errno=%d", errno);
        return MF_OPEN_FAILED;
    }


    chpool_t *chpool = chp_init(fd, PROT_READ | PROT_WRITE);
    if(!chpool)
    {
        log_write(ERROR, "mf_open: can't init chunk pool");
        return MF_OPEN_FAILED;
    }


    log_write(INFO, "mf_open: finished");
    return (mf_handle_t)chpool;
}



int mf_close(mf_handle_t mf)
{
    if(!mf)
        return -1;
    log_write(INFO, "mf_close: started");


    chpool_t *chpool = (chpool_t *)mf;
    int error = chp_deinit(chpool);
    if(error)
    {
        errno = error;
        return -1;
    }


    if(close(chpool->fd) == -1)
    {
        log_write(ERROR, "mf_close: can't close the file");
        return -1;
    }
    free(chpool);


    log_write(INFO, "mf_close: finished");
    return 0;
}



ssize_t mf_read(mf_handle_t mf, void *buf, size_t count, off_t offset)
{
    off_t file_size = mf_file_size(mf);
    if(file_size == -1)
    {
        log_write(ERROR, "mf_read: can't get file size");
        return -1;
    }


    if(mf == MF_OPEN_FAILED || !buf || offset < 0 || offset > file_size)
    {
        errno = EINVAL;
        log_write(ERROR, "mf_read: bad arguments");
        return -1;
    }
    log_write(INFO, "mf_read: started");


    off_t index, len;
    mf_get_ial(offset, count, &index, &len);


    chunk_t *read_chunk;
    int error = chp_find((chpool_t *)mf, index, len, &read_chunk);
    if(error == ENOKEY)
    {
        read_chunk = ch_init(index, len, (chpool_t *)mf);
        if(!read_chunk)
        {
            log_write(ERROR, "mf_read: can't init new chunk for read");
            return -1;
        }
    }
    else if(error)
    {
        log_write(ERROR, "mf_read: chp_find returns error=%d", error);
        return -1;
    }


    void *src = (void *)(((off_t)(read_chunk->data))
                    + (offset - read_chunk->index * get_chunk_size(1)));
    buf = memcpy(buf, src, count);


    log_write(INFO, "mf_read: finished");
    return count;
}



ssize_t mf_write(mf_handle_t mf, const void *buf, size_t count, off_t offset)
{
    off_t file_size = mf_file_size(mf);
    if(file_size == -1)
    {
        log_write(ERROR, "mf_write: can't get file size");
        return -1;
    }


    if(mf == MF_OPEN_FAILED || !buf || offset < 0 || offset > file_size)
    {
        errno = EINVAL;
        log_write(ERROR, "mf_write: bad arguments");
        return -1;
    }
    log_write(DEBUG, "mf_write: started");


    off_t index, len;
    mf_get_ial(offset, count, &index, &len);


    chunk_t *write_chunk;
    int error = chp_find((chpool_t *)mf, index, len, &write_chunk);
    if(error == ENOKEY)
    {
        write_chunk = ch_init(index, len, (chpool_t *)mf);
        if(!write_chunk)
        {
            log_write(ERROR, "mf_write: can't init new chunk for read");
            return -1;
        }
    }
    else if(error)
    {
        log_write(ERROR, "mf_write: chp_find returns error=%d", error);
        return -1;
    }


    void *dst = (void *)(((off_t)(write_chunk->data))
                + (offset - write_chunk->index * get_chunk_size(1)));
    dst = memcpy(dst, buf, count);


    log_write(INFO, "mf_write: finished");
    return count;
}



void *mf_map(mf_handle_t mf, off_t offset, size_t size,
             mf_mapmem_handle_t *mapmem_handle)
{
    if(mf == MF_OPEN_FAILED || offset < 0 || offset + size > mf_file_size(mf))
    {
        errno = EINVAL;
        log_write(ERROR, "mf_map: bad input");
        return NULL;
    }
    log_write(INFO, "mf_map: started");


    if(!size)
        return NULL;


    off_t index, len;
    mf_get_ial(offset, size, &index, &len);


    chunk_t **chunk = (chunk_t **)mapmem_handle;
    int error = chp_find((chpool_t *)mf, index, len, chunk);
    if(error == ENOKEY)
    {
        *chunk = ch_init(index, len, (chpool_t *)mf);
        if(!(*chunk))
        {
            log_write(ERROR, "mf_map: can't init new chunk");
            return NULL;
        }        
    }
    else if(error)
    {
        log_write(ERROR, "mf_map: chp_find returns error=%d", error);
        return NULL;
    }


    (*chunk)->rc ++;


    log_write(INFO, "mf_map: finished");
    return (void *)((*chunk)->data + (offset - (*chunk)->index * get_chunk_size(1)));
}



int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle)
{
    if(mf == MF_OPEN_FAILED || mapmem_handle == MF_MAP_FAILED)
    {
        log_write(ERROR, "mf_unmap: bad arguments");
        errno = EINVAL;
        return -1;
    }
    log_write(INFO, "mf_unmap: started");


    int error = ch_release((chunk_t *)mapmem_handle);
    if(error)
    {
        errno = error;
        log_write(ERROR, "mf_unmap: can't release the chunk, error=%d", error);
        return -1;
    }


    log_write(INFO, "mf_unmap: finished");
    return 0;
}



off_t mf_file_size(mf_handle_t mf)
{
    if(mf == MF_OPEN_FAILED)
    {
        log_write(ERROR, "mf_file_size: bad mf");
        errno = EINVAL;
        return -1;
    }
    log_write(INFO, "mf_file_size: started");


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
        errno = error;
        return -1;
    }


    log_write(INFO,"mf_file_size: finished");
    return sb.st_size;
}
