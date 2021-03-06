﻿#include "chunk_manager.h"



#include <errno.h>
#include <sys/mman.h>
#include <malloc.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>



#include "../logger/logger.h"
#include "../dc_list/dc_list.h"



int PAGESIZE = 0;



static int ch_fill(off_t index, off_t len, chpool_t *chpool, chunk_t *chunk);
static int chp_fill(int fd, int prot, chpool_t *chpool);
static int chp_get_free_chptr(chpool_t *chpool, chunk_t **chunk);
static int chp_add_loaf(chpool_t *chpool);
static int loafs_deinit(chpool_t *chpool);
static int chp_fstr_del(chpool_t *chpool, off_t index, off_t len);
static int chp_fstr_add(chpool_t *chpool, off_t index, chunk_t *chunk);
static int chp_fstr_find(chpool_t *chpool, off_t index, off_t len, chunk_t **chunk);
static int chp_fstr_deinit(chpool_t *chpool);
static fstruct_t *chp_fstr_init();



chunk_t *ch_init(off_t index, off_t len, chpool_t *chpool)
{
    if(!chpool)
        return NULL;
    //log_write(INFO, "ch_init: started");
    int error = 0;


    chunk_t *new_chunk;    
    error = chp_fstr_find(chpool, index, len, &new_chunk);


    if(!error)
    {
        if(new_chunk->rc == 0)
        {
            pthread_mutex_lock(&chpool->zl_write_lock);
            error = dcl_del_by_value(chpool->zero_list, (lvalue_t)new_chunk);
            pthread_mutex_unlock(&chpool->zl_write_lock);
        }

        new_chunk->rc ++;
        //log_write(INFO, "ch_init: finished");
        return new_chunk;
    }
    else if(error && error != ENOKEY)
    {
        //log_write(ERROR, "ch_init: can't add a chunk to set, error=%d", error);
        return NULL;
    }


    error = chp_get_free_chptr(chpool, &new_chunk);
    if(error)
    {
        //log_write(ERROR, "ch_init: can't get free chunk, error=%d", error);
        return NULL;
    }


    error = ch_fill(index, len, chpool, new_chunk);
    if(error)
    {
        //log_write(ERROR, "ch_init: can't fill new chunk, error=%d", error);
        return NULL;
    }


    //log_write(INFO, "ch_init: finished");
    return new_chunk;
}



int ch_deinit(chunk_t *chunk)
{
    if(!chunk)
        return EINVAL;
    //log_write(INFO, "ch_deinit: started");
    int error = 0;
    chpool_t *chpool = chunk->chpool;


    if(!chpool)
        return 0;


    pthread_mutex_lock(&chpool->flock);
    if(munmap(chunk->data, get_chunk_size(chunk->len)) == -1)
    {
        pthread_mutex_unlock(&chpool->flock);
        //log_write(WARNING, "ch_deinit: can't unmap memory");
        return errno;
    }
    pthread_mutex_unlock(&chpool->flock);


    error = chp_fstr_del(chunk->chpool, chunk->index, chunk->len);
    if(error)
    {
        //log_write(WARNING, "ch_deinit: can't delete chunk from set");
        return error;
    }


    if(chunk->rc == 0)
    {
        pthread_mutex_lock(&chpool->zl_write_lock);
        error = dcl_del_by_value(chunk->chpool->zero_list, (lvalue_t)chunk);
        pthread_mutex_unlock(&chpool->zl_write_lock);
        if(error == ENOKEY)
        {
            //log_write(WARNING, "ch_deinit: it seems that another thread has already delete the chunk");
            return error;
        }
        else if(error)
        {
            //log_write(WARNING, "ch_deinit: can't delete chunk from zero list");
            return error;
        }
    }


    chunk->rc = 0;
    chunk->len = 0;
    chunk->index = 0;
    chunk->data = NULL;


    pthread_mutex_lock(&chpool->fl_write_lock);
    error = dcl_add_last(chunk->chpool->free_list, (lvalue_t)chunk);
    pthread_mutex_unlock(&chpool->fl_write_lock);
    if(error)
    {
        //log_write(WARNING, "ch_deinit: can't add chunk to free list");
        return error;
    }


    return 0;
}



int ch_release(chunk_t *chunk)
{
    if(!chunk)
        return EINVAL;
    log_write(INFO, "ch_release: started");
    int error = 0;


    if(chunk->rc == 0)
        return EAGAIN;


    if(--(chunk->rc) != 0)
        return 0;

    chpool_t *chpool = chunk->chpool;
    pthread_mutex_lock(&chpool->zl_write_lock);
    error = dcl_add_last(chunk->chpool->zero_list, (lvalue_t)chunk);
    pthread_mutex_unlock(&chpool->zl_write_lock);
    if(error)
    {
        //log_write(ERROR, "ch_release: can't add chunk to the zero list");
        return error;
    }


    //log_write(INFO, "ch_release: finished");
    return 0;
}



size_t get_chunk_size(off_t multiplier)
{
    if(!PAGESIZE)
        PAGESIZE = sysconf(_SC_PAGESIZE);
    return multiplier * PAGESIZE;
}



static int ch_fill(off_t index, off_t len, chpool_t *chpool, chunk_t *chunk)
{
    int error = 0;  
    off_t mmap_len = MIN(len * 4,
                         (chpool->file_size - get_chunk_size(index)) /
                          chpool->page_size + 1);

    chunk->rc = 1;
    chunk->len = mmap_len;
    chunk->index = index;
    chunk->chpool = chpool;

    pthread_mutex_lock(&chpool->flock);
    chunk->data = mmap(NULL, get_chunk_size(mmap_len), chpool->prot,
                       MAP_SHARED, chpool->fd, get_chunk_size(index));
    pthread_mutex_unlock(&chpool->flock);
    while(chunk->data == MAP_FAILED && errno == ENOMEM)
    {
        chunk_t *del_chunk;
        if(chpool->zero_list->size != 0)
        {
            del_chunk = (chunk_t *)chpool->zero_list->head->value;
            error = ch_deinit(del_chunk);
        }
        else if(mmap_len > len)
        {
            mmap_len = len;
            chunk->len = mmap_len;
        }
        else
        {
            //log_write(WARNING, "ch_init: no free memory, all chunks are used");
            return ENOMEM;
        }

        pthread_mutex_lock(&chpool->flock);
        chunk->data = mmap(NULL, get_chunk_size(mmap_len), chpool->prot,
                           MAP_SHARED, chpool->fd, get_chunk_size(index));
        pthread_mutex_unlock(&chpool->flock);
    }
    if(chunk->data == MAP_FAILED)
    {
        //log_write(ERROR, "ch_init: mmap failed, errno=%d", errno);
        return errno;
    }


    return chp_fstr_add(chpool, index, chunk);
}

//*****************************************************************************
chpool_t *chp_init(int fd, int prot)
{    
    if(fd < 0 || (!(prot & PROT_READ) && !(prot & PROT_WRITE)) )
    {
        //log_write(ERROR, "chp_init: invaid input");
        return NULL;
    }
    log_write(DEBUG, "chp_init: started");


    chpool_t *chpool = (chpool_t *)calloc(1, sizeof(chpool_t));
    if(!chpool)
    {
        //log_write(ERROR, "chp_init: chpool calloc failed");
        return NULL;
    }


    int error = chp_fill(fd, prot, chpool);
    if(error)
    {
        //log_write(ERROR, "chp_init: can't fill chpool");
        return NULL;
    }


    //log_write(DEBUG, "chp_init: finished");
    return chpool;
}



int chp_deinit(chpool_t *chpool)
{
    if(!chpool)
        return EINVAL;
    //log_write(INFO, "chp_deinit: started");
    int error = 0;


    //int error = loafs_deinit(chpool);
    if(error)
    {
        //log_write(ERROR, "chp_deinit: can't deinit loafs");
        return error;
    }


    error = dcl_deinit(chpool->zero_list);
    if(error)
    {
        //log_write(ERROR, "chp_deinit: can't deinit the zero list");
        return error;
    }


    error = dcl_deinit(chpool->free_list);
    if(error)
    {
        //log_write(ERROR, "chp_deinit: can't deinit the free list");
        return error;
    }


    error = chp_fstr_deinit(chpool);
    if(error)
    {
        //log_write(ERROR, "chp_deinit: can't deinit the set");
        return error;
    }


    //log_write(INFO, "chp_deinit: finished");
    return 0;
}



static int chp_fill(int fd, int prot, chpool_t *chpool)
{
    chpool->free_list = dcl_init();
    chpool->zero_list = dcl_init();
    chpool->fstr = chp_fstr_init();
    chpool->pool = (chunk_t **)calloc(1, sizeof(chunk_t *));
    *(chpool->pool) = (chunk_t *)calloc(DEFAULT_ARRAY_SIZE, sizeof(chunk_t));

    if(!chpool->free_list || !chpool->zero_list || !chpool->fstr
                          || !chpool->pool || !(*(chpool->pool)))
    {
        //log_write(ERROR, "chp_init: chunk containing structs init has failed");
        return NULL;
    }


    chpool->is_all_map = 0;
    chpool->arrays_cnt = 1;
    chpool->fd = fd;
    chpool->prot = prot;
    chpool->file_size = chp_file_size(fd);
    chpool->page_size = sysconf(_SC_PAGESIZE);


    pthread_mutex_lock(&chpool->fl_write_lock);
    for(int i = 0; i < DEFAULT_ARRAY_SIZE; i++)
    {
        chunk_t *curr_chunk = &((*chpool->pool)[i]);
        if(dcl_add_last(chpool->free_list, (lvalue_t)curr_chunk))
        {
            pthread_mutex_unlock(&chpool->fl_write_lock);
            //log_write(ERROR, "chp_init: can't add free chunk to list");
            return NULL;
        }
    }
    pthread_mutex_unlock(&chpool->fl_write_lock);
    return 0;
}



static int chp_get_free_chptr(chpool_t *chpool, chunk_t **chunk)
{
    pthread_mutex_lock(&chpool->fl_write_lock);
    if(chpool->free_list->size)
    {        
        *chunk = chpool->free_list->head->value;
        int error = dcl_del_first(chpool->free_list);
        pthread_mutex_unlock(&chpool->fl_write_lock);

        return error;
    }
    pthread_mutex_unlock(&chpool->fl_write_lock);


    pthread_mutex_lock(&chpool->zl_write_lock);
    if(chpool->zero_list->size)
    {
        *chunk = chpool->zero_list->head->value;
        pthread_mutex_unlock(&chpool->zl_write_lock);
        ch_deinit(*chunk);

        pthread_mutex_lock(&chpool->fl_write_lock);
        int error = dcl_del_first(chpool->free_list);
        pthread_mutex_unlock(&chpool->fl_write_lock);

        return error;
    }
    pthread_mutex_unlock(&chpool->zl_write_lock);


    int error = chp_add_loaf(chpool);
    if(error)
    {
        //log_write(ERROR, "chp_get_free_chptr: can't add new loaf");
        return error;
    }


    pthread_mutex_lock(&chpool->fl_write_lock);
    *chunk = chpool->free_list->head->value;
    error = dcl_del_first(chpool->free_list);
    pthread_mutex_unlock(&chpool->fl_write_lock);

    return error;
}



static int chp_add_loaf(chpool_t *chpool)
{
    chpool->pool = (chunk_t **)realloc((void *)chpool->pool,
                   (chpool->arrays_cnt + 1) * sizeof(chunk_t *));
    if(!chpool->pool)
    {
        //log_write(ERROR, "chp_add_loaf: can't reallocate cell for new loaf");
        return ENOMEM;
    }


    chpool->pool[chpool->arrays_cnt++] = (chunk_t *)calloc(DEFAULT_ARRAY_SIZE,
                                                               sizeof(chunk_t));
    if(!chpool->pool[chpool->arrays_cnt - 1])
    {
        //log_write(ERROR, "chp_add_loaf: can't allocate new loaf");
        return ENOMEM;
    }


    pthread_mutex_lock(&chpool->fl_write_lock);
    for(int i = 0; i < DEFAULT_ARRAY_SIZE; i++)
    {
        chunk_t *curr_chunk = &((*chpool->pool)[i]);
        if(dcl_add_last(chpool->free_list, (lvalue_t)curr_chunk))
        {
            pthread_mutex_unlock(&chpool->fl_write_lock);
            //log_write(ERROR, "chp_add_loaf: can't add free chunk to the list");
            return -1;
        }
    }
    pthread_mutex_unlock(&chpool->fl_write_lock);

    return 0;
}



static int loafs_deinit(chpool_t *chpool)
{
    int error = 0;
    if(chpool->all_file)
    {
        error = ch_deinit(chpool->all_file);
        if(error)
            //log_write(ERROR, "can't deinit all file chunk");

        return error;
    }



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


off_t chp_file_size(int fd)
{
    if(fd < 0)
    {
        //log_write(ERROR, "ch_file_size: bad file descriptor");
        return -1;
    }


    struct stat sb = {0};
    int error = fstat(fd, &sb);
    if(error)
    {
        //log_write(ERROR, "ch_file_size: fd returns error=%d", error);
        errno = error;
        return -1;
    }


    //log_write(INFO,"ch_file_size: finished");
    return sb.st_size;
}



#if (SKIPLIST_ON == 0)
static int chp_fstr_find(chpool_t *chpool, off_t index, off_t len, chunk_t **chunk)
{
    if(!chpool || !chunk)
        return EINVAL;

    pthread_mutex_lock(&chpool->fstr_write_lock);
    chpool->fstr_reads_numb++;
    pthread_mutex_unlock(&chpool->fstr_write_lock);
    int error = ht_find(chpool->fstr, index, len, chunk);
    chpool->fstr_reads_numb--;

    return error;
}

static int chp_fstr_del(chpool_t *chpool, off_t index, off_t len)
{
    pthread_mutex_lock(&chpool->fstr_write_lock);
    while(chpool->fstr_reads_numb)
        continue;
    int error = ht_del(chpool->fstr, index, len);
    pthread_mutex_unlock(&chpool->fstr_write_lock);

    return error;
}

static int chp_fstr_add(chpool_t *chpool, off_t index, chunk_t *chunk)
{
    pthread_mutex_lock(&chpool->fstr_write_lock);
    while(chpool->fstr_reads_numb)
        continue;
    int error = ht_add(chpool->fstr, (hkey_t)index, (hvalue_t)chunk);
    pthread_mutex_unlock(&chpool->fstr_write_lock);
}

static fstruct_t *chp_fstr_init()
{
    return ht_init(DEFAULT_HT_SIZE, hash_fnv1a);
}

static int chp_fstr_deinit(chpool_t *chpool)
{
    return ht_deinit(chpool->fstr);
}
#else
static int chp_fstr_find(chpool_t *chpool, off_t index, off_t len, chunk_t **chunk)
{
    if(!chpool || !chunk)
        return EINVAL;

    pthread_mutex_lock(&chpool->fstr_write_lock);
    chpool->fstr_reads_numb++;
    pthread_mutex_unlock(&chpool->fstr_write_lock);
    int error = skiplist_find((skiplist_t *)chpool->fstr, index, len, chunk);
    chpool->fstr_reads_numb--;

    return error;
}

static int chp_fstr_del(chpool_t *chpool, off_t index, off_t len)
{
    pthread_mutex_lock(&chpool->fstr_write_lock);
    while(chpool->fstr_reads_numb)
        continue;
    int error = skiplist_del((skiplist_t *)chpool->fstr, index);
    pthread_mutex_unlock(&chpool->fstr_write_lock);

    return error;
}

static int chp_fstr_add(chpool_t *chpool, off_t index, chunk_t *chunk)
{
    pthread_mutex_lock(&chpool->fstr_write_lock);
    while(chpool->fstr_reads_numb)
        continue;
    int error = skiplist_add((skiplist_t *)chpool->fstr, index, chunk);
    pthread_mutex_unlock(&chpool->fstr_write_lock);
}

static fstruct_t *chp_fstr_init()
{
    return skiplist_init();
}

static int chp_fstr_deinit(chpool_t *chpool)
{
    return skiplist_deinit(chpool->fstr);
}

#endif
