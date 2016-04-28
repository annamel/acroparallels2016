#include "chunk_manager.h"

#include <errno.h>
#include <sys/mman.h>
#include <malloc.h>
#include <unistd.h>
#include <stdbool.h>



#include "logger/logger.h"
#include "dc_list/dc_list.h"
#include "hash_table/hash_table.h"



static int chp_add_loaf(chpool_t *chpool);
static int chp_get_free_chptr(chpool_t *chpool, chunk_t **new_chunk);



chunk_t *ch_init(off_t index, off_t len, chpool_t *chpool)
{
    if(!chpool)
    {
        log_write(ERROR, "ch_init(index=%d, len=%d): invaid input",
                  index, len);
        return NULL;
    }
    log_write(DEBUG, "ch_init(index=%d, len=%d): started", index, len);

    chunk_t *new_chunk;
    int error = chp_get_free_chptr(chpool, &new_chunk);
    if(error)
    {
        log_write(ERROR, "ch_init(index=%d, len=%d: can't get free chunk, error=%d",
                  index, len, error);
        return NULL;
    }

    new_chunk->data = mmap(NULL, get_chunk_size(len), chpool->prot,
                           MAP_SHARED, chpool->fd, get_chunk_size(index));
    if(new_chunk->data == MAP_FAILED)
    {
        log_write(ERROR, "ch_init(index=%d, len=%d): mmap failed, errno=%d",
                  index, len, errno);
        printf("errno=%d\n", errno);
        return NULL;
    }

    new_chunk->rc = 1;
    new_chunk->len = len;
    new_chunk->index = index;
    new_chunk->chpool = chpool;

    error = ht_add_item(chpool->ht, (hkey_t)index, (hvalue_t)new_chunk);
    if(error)
    {
        log_write(ERROR, "ch_init(index=%d, len=%d): can't add chunk to hash table, error=%d",
                  index, len, error);
        return NULL;
    }

    log_write(DEBUG, "ch_init(index=%d, len=%d): finished", index, len);

    return new_chunk;
}



size_t get_chunk_size(off_t multiplier)
{
    return multiplier * sysconf(_SC_PAGESIZE);
}



















//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
chpool_t *chp_init(int fd, int prot)
{
    if(fd < 0 || (!(prot & PROT_READ) && !(prot & PROT_WRITE)) )
    {
        log_write(ERROR, "chp_init(fd=%d, prot=%d): invaid input",
                  fd, prot);
        return NULL;
    }
    log_write(DEBUG, "chp_init(fd=%d, prot=%d): started",
              fd, prot);

    chpool_t *new_chpool = (chpool_t *)calloc(1, sizeof(chpool_t));
    if(!new_chpool)
    {
        log_write(ERROR, "chp_init(fd=%d, prot=%d): calloc failed",
                  fd, prot);
        return NULL;
    }

    new_chpool->fd = fd;
    new_chpool->prot = prot;
    new_chpool->free_list = dcl_init();
    if(!new_chpool->free_list)
    {
        log_write(ERROR, "chp_init(fd=%d, prot=%d): free list init failed",
                  fd, prot);
        return NULL;
    }

    new_chpool->zero_list = dcl_init();
    if(!new_chpool->zero_list)
    {
        log_write(ERROR, "chp_init(fd=%d, prot=%d): zero list init failed",
                  fd, prot);
        return NULL;
    }
    new_chpool->ht = ht_init(DEFAULT_HASHTABLE_SIZE, hash_fnv1a);
    if(!new_chpool->ht)
    {
        log_write(ERROR, "chp_init(fd=%d, prot=%d): hash table init failed",
                  fd, prot);
        return NULL;
    }

    new_chpool->pool = (chunk_t **)calloc(1, sizeof(chunk_t *));
    new_chpool->arrays_cnt = 1;
    *(new_chpool->pool) = (chunk_t *)calloc(DEFAULT_ARRAY_SIZE, sizeof(chunk_t));
    if(!new_chpool->pool || !*new_chpool->pool)
    {
        log_write(ERROR, "chp_init(fd=%d, prot=%d): pool init failed",
                  fd, prot);
        return NULL;
    }

    for(int i = 0; i < DEFAULT_ARRAY_SIZE; i++)
        if(dcl_add_last(new_chpool->free_list, (lvalue_t)&((*new_chpool->pool)[i])))
        {
            log_write(ERROR, "chp_init(fd=%d, prot=%d): can't add free chunk to list",
                      fd, prot);
            return NULL;
        }

    log_write(DEBUG, "chp_init(fd=%d, prot=%d): finished",
              fd, prot);

    return new_chpool;
}



int chp_find(chpool_t *chpool, off_t index, off_t len, chunk_t **chunk)
{
    if(!chpool)
        return EINVAL;

    log_write(DEBUG, "chp_find(index=%d, len=%d): started", index, len);

    hkey_t key = (hkey_t)index;

    int idx = chpool->ht->hash_func(key, HASH_CONST_2) % chpool->ht->size;
    item_t *item_ptr = chpool->ht->table[idx];
    if(!item_ptr)
        return ENOKEY;

    while(item_ptr)
    {
        if(item_ptr->key <= key &&
          ((chunk_t *)item_ptr->value)->len >= len + (key - item_ptr->key))
        {
            *chunk = (chunk_t *)item_ptr->value;
            return 0;
        }
        else
        {
            item_ptr = item_ptr->next;
            continue;
        }
    }

    log_write(DEBUG, "chp_find(index=%d, len=%d): finished", index, len);

    return ENOKEY;
}



int chp_chunk_release(chunk_t *chunk)
{
    if(!chunk)
        return EINVAL;

    log_write(DEBUG, "chp_chunk_release: started");

    if(chunk->rc == 0)
    {
        log_write(WARNING, "chp_chunk_release: trying to release chunk with zero rc");
        return EAGAIN;
    }

    if(--chunk->rc == 0)
    {
        int error = munmap(chunk->data, get_chunk_size(chunk->len));
        if(error == -1)
        {
            log_write(WARNING, "chp_chunk_release: can't unmap memory");
            return errno;
        }

        chunk->rc = 0;
        chunk->len = 0;
        chunk->index = 0;
        chunk->data = NULL;

        error = ht_del_item_by_kav(chunk->chpool->ht,
                                   (hkey_t)chunk->index, (hvalue_t)chunk);
        if(error)
        {
            log_write(WARNING, "chp_chunk_release: can't delete chunk from hash table, error=%d",
                      error);
            return error;
        }

        error = dcl_add_last(chunk->chpool->free_list, (lvalue_t)chunk);
        if(error)
        {
            log_write(WARNING, "chp_chunk_release: can't add chunk to free list, error=%d",
                      error);
            return error;
        }
    }

    log_write(DEBUG, "chp_chunk_release: finished");

    return 0;
}



static int chp_add_loaf(chpool_t *chpool)
{
    if(!chpool)
        return EINVAL;

    log_write(INFO, "chp_add_loaf: started");

    chpool->pool = realloc(chpool->pool, chpool->arrays_cnt + 1);
    if(!chpool->pool)
    {
        log_write(ERROR, "chp_add_loaf: can't reallocate cell for new loaf");
        return ENOMEM;
    }

    chpool->pool[chpool->arrays_cnt++] = (chunk_t *)calloc(DEFAULT_ARRAY_SIZE,
                                                               sizeof(chunk_t));
    if(!chpool->pool[chpool->arrays_cnt-1])
    {
        log_write(ERROR, "chp_add_loaf: can't allocate new loaf");
        return ENOMEM;
    }

    for(int i = 0; i < DEFAULT_ARRAY_SIZE; i++)
        if(dcl_add_last(chpool->free_list, (lvalue_t)&((*chpool->pool)[i])))
        {
            log_write(ERROR, "chp_add_loaf: can't add free chunk to list");
            return NULL;
        }

    log_write(INFO, "chp_add_loaf: finished");
    return 0;
}



static int chp_get_free_chptr(chpool_t *chpool, chunk_t **new_chunk)
{
    if(!chpool)
        return EINVAL;

    log_write(INFO, "chp_get_free_chptr: started");

    if(!chpool->free_list->size)
    {
        log_write(INFO, "chp_get_free_chptr: free_list is empty");

        if(!chpool->zero_list->size)
        {
            log_write(INFO, "chp_get_free_chptr: zero_list also is empty");

            chp_add_loaf(chpool);

            *new_chunk = chpool->free_list->head->value;
            dcl_del_first(chpool->free_list);
        }
        else
        {
            *new_chunk = chpool->zero_list->head->value;
            if(munmap((*new_chunk)->data, get_chunk_size((*new_chunk)->len)) == -1)
            {
                log_write(ERROR,"chp_get_free_chptr: can't unmap memory");
                return NULL;
            }

            dcl_del_first(chpool->zero_list);
        }
    }
    else
    {
        *new_chunk = chpool->free_list->head->value;
        dcl_del_first(chpool->free_list);
    }

    log_write(INFO, "chp_get_free_chptr: finished");
    return 0;
}


