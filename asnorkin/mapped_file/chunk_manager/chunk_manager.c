#include "chunk_manager.h"



#include <errno.h>
#include <sys/mman.h>
#include <malloc.h>
#include <unistd.h>
#include <stdbool.h>



#include "../logger/logger.h"
#include "../dc_list/dc_list.h"



static int chp_get_free_chptr(chpool_t *chpool, chunk_t **chunk);
static int chp_add_loaf(chpool_t *chpool);
static int loafs_deinit(chpool_t *chpool);



chunk_t *ch_init(off_t index, off_t len, chpool_t *chpool)
{
    if(!chpool)
    {
        log_write(ERROR, "ch_init: chpool in NULL");
        return NULL;
    }
    log_write(INFO, "ch_init: started");


    chunk_t *new_chunk;
    int error = chp_get_free_chptr(chpool, &new_chunk);
    if(error)
    {
        log_write(ERROR, "ch_init: can't get free chunk, error=%d", error);
        return NULL;
    }


    new_chunk->data = mmap(NULL, get_chunk_size(len), chpool->prot,
                           MAP_SHARED, chpool->fd, get_chunk_size(index));
    if(new_chunk->data == MAP_FAILED)
    {
        log_write(ERROR, "ch_init: mmap failed, errno=%d", errno);
        return NULL;
    }


    new_chunk->rc = 1;
    new_chunk->len = len;
    new_chunk->index = index;
    new_chunk->chpool = chpool;
    error = ss_add(chpool->sset, (chunk_t *)new_chunk);
    if(error)
    {
        log_write(ERROR, "ch_init: can't add a chunk to set, error=%d", error);
        return NULL;
    }


    log_write(INFO, "ch_init: finished");
    return new_chunk;
}



int ch_deinit(chunk_t *chunk)
{
    if(!chunk)
        return EINVAL;
    log_write(INFO, "ch_deinit: started");
    int error = 0;


    if(!chunk->chpool)
        return 0;


    if(chunk->rc)
        error = ss_del(chunk->chpool->sset, chunk->index, chunk->len);
    if(error)
    {
        log_write(WARNING, "ch_deinit: can't delete chunk from set");
        return error;
    }


    if(chunk->rc == 0)
    {
        error = dcl_del_by_value(chunk->chpool->zero_list, (lvalue_t)chunk);
        if(error)
        {
            log_write(ERROR, "ch_deinit: can't delete chunk from zero list");
            return error;
        }
    }


    if(munmap(chunk->data, get_chunk_size(chunk->len)) == -1)
    {
        log_write(WARNING, "chp_chunk_release: can't unmap memory");
        return errno;
    }

    chunk->rc = 0;
    chunk->len = 0;
    chunk->index = 0;
    chunk->data = NULL;


    error = dcl_add_last(chunk->chpool->free_list, (lvalue_t)chunk);
    if(error)
    {
        log_write(WARNING, "chp_chunk_release: can't add chunk to free list");
        return error;
    }


    return 0;
}



int ch_release(chunk_t *chunk)
{
    if(!chunk)
        return EINVAL;
    log_write(INFO, "ch_release: started");


    if(chunk->rc == 0)
        return EAGAIN;


    if(--(chunk->rc) != 0)
        return 0;


    int error = dcl_add_last(chunk->chpool->zero_list, (lvalue_t)chunk);
    if(error)
    {
        log_write(ERROR, "ch_release: can't add chunk to the zero list");
        return error;
    }


    log_write(INFO, "ch_release: finished");
    return 0;
}



size_t get_chunk_size(off_t multiplier)
{
    return multiplier * sysconf(_SC_PAGESIZE);
}



//*****************************************************************************
chpool_t *chp_init(int fd, int prot)
{
    if(fd < 0 || (!(prot & PROT_READ) && !(prot & PROT_WRITE)) )
    {
        log_write(ERROR, "chp_init: invaid input");
        return NULL;
    }
    log_write(DEBUG, "chp_init: started");


    chpool_t *chpool = (chpool_t *)calloc(1, sizeof(chpool_t));
    if(!chpool)
    {
        log_write(ERROR, "chp_init: chpool calloc failed");
        return NULL;
    }


    chpool->free_list = dcl_init();
    chpool->zero_list = dcl_init();
    chpool->sset = ss_init();
    chpool->pool = (chunk_t **)calloc(1, sizeof(chunk_t *));
    *(chpool->pool) = (chunk_t *)calloc(DEFAULT_ARRAY_SIZE, sizeof(chunk_t));
    if(!chpool->free_list || !chpool->zero_list || !chpool->sset
                          || !chpool->pool || !(*(chpool->pool)))
    {
        log_write(ERROR, "chp_init: chunk containing structs init has failed");
        return NULL;
    }


    chpool->arrays_cnt = 1;
    chpool->fd = fd;
    chpool->prot = prot;


    for(int i = 0; i < DEFAULT_ARRAY_SIZE; i++)
    {
        chunk_t *curr_chunk = &((*chpool->pool)[i]);
        if(dcl_add_last(chpool->free_list, (lvalue_t)curr_chunk))
        {
            log_write(ERROR, "chp_init: can't add free chunk to list");
            return NULL;
        }
    }


    log_write(DEBUG, "chp_init: finished");
    return chpool;
}



int chp_deinit(chpool_t *chpool)
{
    if(!chpool)
        return EINVAL;
    log_write(INFO, "chp_deinit: started");


    int error = loafs_deinit(chpool);
    if(error)
    {
        log_write(ERROR, "chp_deinit: can't deinit loafs");
        return error;
    }


    error = dcl_deinit(chpool->zero_list);
    if(error)
    {
        log_write(ERROR, "chp_deinit: can't deinit the zero list");
        return error;
    }


    error = dcl_deinit(chpool->free_list);
    if(error)
    {
        log_write(ERROR, "chp_deinit: can't deinit the free list");
        return error;
    }


    error = ss_deinit(chpool->sset);
    if(error)
    {
        log_write(ERROR, "chp_deinit: can't deinit the set");
        return error;
    }


    log_write(INFO, "chp_deinit: finished");
    return 0;
}



int chp_find(chpool_t *chpool, off_t index, off_t len, chunk_t **chunk)
{
    if(!chpool || !chunk)
        return EINVAL;


    return ss_find(chpool->sset, index, len, chunk);
}



static int chp_get_free_chptr(chpool_t *chpool, chunk_t **chunk)
{
    if(chpool->free_list->size)
    {
        *chunk = chpool->free_list->head->value;
        return dcl_del_first(chpool->free_list);
    }


    if(chpool->zero_list->size)
    {
        *chunk = chpool->zero_list->head->value;
        if(munmap((*chunk)->data, get_chunk_size((*chunk)->len)) == -1)
        {
            log_write(ERROR, "chp_get_free_chptr: can't unmap memory");
            return -1;
        }
        return dcl_del_first(chpool->zero_list);
    }


    int error = chp_add_loaf(chpool);
    if(error)
    {
        log_write(ERROR, "chp_get_free_chptr: can't add new loaf");
        return error;
    }
    *chunk = chpool->free_list->head->value;

    return dcl_del_first(chpool->free_list);
}



static int chp_add_loaf(chpool_t *chpool)
{
    chpool->pool = (chunk_t **)realloc((void *)chpool->pool,
                   (chpool->arrays_cnt + 1) * sizeof(chunk_t *));
    if(!chpool->pool)
    {
        log_write(ERROR, "chp_add_loaf: can't reallocate cell for new loaf");
        return ENOMEM;
    }


    chpool->pool[chpool->arrays_cnt++] = (chunk_t *)calloc(DEFAULT_ARRAY_SIZE,
                                                               sizeof(chunk_t));
    if(!chpool->pool[chpool->arrays_cnt - 1])
    {
        log_write(ERROR, "chp_add_loaf: can't allocate new loaf");
        return ENOMEM;
    }


    for(int i = 0; i < DEFAULT_ARRAY_SIZE; i++)
    {
        chunk_t *curr_chunk = &((*chpool->pool)[i]);
        if(dcl_add_last(chpool->free_list, (lvalue_t)curr_chunk))
        {
            log_write(ERROR, "chp_add_loaf: can't add free chunk to the list");
            return -1;
        }
    }

    return 0;
}



static int loafs_deinit(chpool_t *chpool)
{
    int error = 0;
    for(unsigned int i = 0; i < chpool->arrays_cnt; i++)
    {
       for(int j = 0; j < DEFAULT_ARRAY_SIZE; j++)
       {
           chunk_t *curr_chunk = &(chpool->pool[i][j]);
           error = ch_deinit(curr_chunk);
           if(error)
               return error;           
       }

       free((chpool->pool)[i]);
       chpool->pool[i] = NULL;
    }

    free(chpool->pool);
    chpool->pool = NULL;
    return 0;
}
