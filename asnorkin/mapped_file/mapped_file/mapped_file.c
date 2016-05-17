#include "mapped_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>



#include "../chunk_manager/chunk_manager.h"
#include "../logger/logger.h"
#include "../typedefs.h"



static int mf_map_all(chpool_t *chpool);



static int mf_get_ial(off_t offset, size_t size, off_t* index, off_t* len, off_t pgsz)
{
    off_t pa_offset = offset & ~(pgsz - 1);
    off_t new_size = size + offset - pa_offset;

    *index = pa_offset / pgsz;
    *len = new_size / pgsz;
    if(new_size % pgsz) (*len)++;

    return 0;
}



mf_handle_t mf_open(const char *pathname)
{
    log_write(INFO, "mf_open: started");
    if(!pathname)
    {
        //log_write(ERROR, "mf_open: bad filename");
        errno = EINVAL;
        return MF_OPEN_FAILED;
    }    


    int fd = open(pathname, O_RDWR, 0666);
    if(fd == -1)
    {
        //log_write(ERROR, "mf_open: can't open file, errno=%d", errno);
        return MF_OPEN_FAILED;
    }


    chpool_t *chpool = chp_init(fd, PROT_READ | PROT_WRITE);
    if(!chpool)
    {
        //log_write(ERROR, "mf_open: can't init chunk pool");
        return MF_OPEN_FAILED;
    }


    //  Trying to mmap all file
    /*if(!mf_map_all(chpool))
        printf("All file mmap OK!");*/


    //log_write(INFO, "mf_open: finished");
    return (mf_handle_t)chpool;
}



int mf_close(mf_handle_t mf)
{
    if(!mf)
        return -1;
    //log_write(INFO, "mf_close: started");


    chpool_t *chpool = (chpool_t *)mf;
    int error = chp_deinit(chpool);
    if(error)
    {
        errno = error;
        return -1;
    }


    if(close(chpool->fd) == -1)
    {
        //log_write(ERROR, "mf_close: can't close the file");
        return -1;
    }
    free(chpool);


    //log_write(INFO, "mf_close: finished");
    return 0;
}



ssize_t mf_read(mf_handle_t mf, void *buf, size_t count, off_t offset)
{
    if(mf == MF_OPEN_FAILED || !buf || offset < 0)
    {
        errno = EINVAL;        
        return -1;
    }
    //log_write(INFO, "mf_read: started");
    if(((chpool_t *)mf)->arrays_cnt == 1 &&
       ((chpool_t *)mf)->free_list->size == DEFAULT_ARRAY_SIZE)
        mf_map_all((chpool_t*)mf);


    off_t pgsz = ((chpool_t *)mf)->page_size;
    off_t flsz = ((chpool_t *)mf)->file_size;
    count = MIN(count, flsz - offset);
    off_t index, len;
    mf_get_ial(offset, count, &index, &len, pgsz);


    chunk_t *read_chunk = ch_init(index, len, (chpool_t *)mf);
    if(!read_chunk)
    {
        //log_write(ERROR, "mf_read: can't init new chunk for read");
        return -1;
    }


    void *src = (void *)(((off_t)(read_chunk->data))
                    + (offset - read_chunk->index * pgsz));
    buf = memcpy(buf, src, count);


    ch_release(read_chunk);


    //log_write(INFO, "mf_read: finished");
    return count;
}



ssize_t mf_write(mf_handle_t mf, const void *buf, size_t count, off_t offset)
{
    if(mf == MF_OPEN_FAILED || !buf || offset < 0)
    {
        errno = EINVAL;        
        return -1;
    }
    //log_write(DEBUG, "mf_write: started");
    if(((chpool_t *)mf)->arrays_cnt == 1 &&
       ((chpool_t *)mf)->free_list->size == DEFAULT_ARRAY_SIZE)
        mf_map_all((chpool_t*)mf);


    off_t pgsz = ((chpool_t *)mf)->page_size;
    off_t flsz = ((chpool_t *)mf)->file_size;
    count = MIN(count, flsz - offset);
    off_t index, len;
    mf_get_ial(offset, count, &index, &len, pgsz);


    chunk_t *write_chunk = ch_init(index, len, (chpool_t *)mf);
    if(!write_chunk)
    {
        //log_write(ERROR, "mf_write: can't init new chunk for read");
        return -1;
    }


    void *dst = (void *)(((off_t)(write_chunk->data))
                + (offset - write_chunk->index * pgsz));
    dst = memcpy(dst, buf, count);


    ch_release(write_chunk);


    //log_write(INFO, "mf_write: finished");
    return count;
}



void *mf_map(mf_handle_t mf, off_t offset, size_t size,
             mf_mapmem_handle_t *mapmem_handle)
{
    if(mf == MF_OPEN_FAILED || offset < 0)
    {
        errno = EINVAL;
        return NULL;
    }
    //log_write(INFO, "mf_map: started");
    if(((chpool_t *)mf)->arrays_cnt == 1 &&
       ((chpool_t *)mf)->free_list->size == DEFAULT_ARRAY_SIZE)
        mf_map_all((chpool_t*)mf);


    if(!size)
        return NULL;


    off_t pgsz = ((chpool_t *)mf)->page_size;
    off_t flsz = ((chpool_t *)mf)->file_size;
    size = MIN(size, flsz - offset);
    off_t index, len;
    mf_get_ial(offset, size, &index, &len, pgsz);


    chunk_t **chunk = (chunk_t **)mapmem_handle;    
    *chunk = ch_init(index, len, (chpool_t *)mf);
    if(!(*chunk))
    {
        //log_write(ERROR, "mf_map: can't init new chunk");
        return NULL;
    }


    //log_write(INFO, "mf_map: finished");
    return (void *)((*chunk)->data +
                   (offset - (*chunk)->index * pgsz));
}



int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle)
{
    if(mf == MF_OPEN_FAILED || mapmem_handle == MF_MAP_FAILED)
    {
        //log_write(ERROR, "mf_unmap: bad arguments");
        errno = EINVAL;
        return -1;
    }
    //log_write(INFO, "mf_unmap: started");


    int error = ch_release((chunk_t *)mapmem_handle);
    if(error)
    {
        errno = error;
        //log_write(ERROR, "mf_unmap: can't release the chunk, error=%d", error);
        return -1;
    }


    //log_write(INFO, "mf_unmap: finished");
    return 0;
}



off_t mf_file_size(mf_handle_t mf)
{
    if(mf == MF_OPEN_FAILED)
    {
        //log_write(ERROR, "mf_file_size: bad mf");
        errno = EINVAL;
        return -1;
    }
    //log_write(INFO, "mf_file_size: started");


    return chp_file_size(((chpool_t *)mf)->fd);
}



static int mf_map_all(chpool_t *chpool)
{
    off_t pgsz = chpool->page_size;
    off_t flsz = chpool->file_size;
    off_t index, len;
    mf_get_ial(0, flsz, &index, &len, pgsz);


    chunk_t *chunk = ch_init(index, len, chpool);
    if(!chunk)
    {
        //log_write(ERROR, "mf_map: can't init new chunk");
        return -1;
    }

    return 0;
}
