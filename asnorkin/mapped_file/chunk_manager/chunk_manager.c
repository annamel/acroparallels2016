#include "chunk_manager.h"


#define DEFAULT_HASHTABLE_SIZE 256



static size_t get_chunk_size(off_t multiplier);
static int chunk_init(chunk_t *chunk, chpool_t *chpool, off_t index, off_t len);
static int chunk_destruct(chunk_t *chunk);
static int chunk_construct(chunk_t *chunk, chpool_t *chpool, off_t index, off_t len);
static int chpool_init(chpool_t *chpool, size_t size, int fd, int prot);
static int chpool_del(chunk_t *chunk);
static int chpool_add(chunk_t *chunk);



static int chunk_init(chunk_t *chunk, chpool_t *chpool, off_t index, off_t len)
{
    chunk->index = index;
    chunk->len = len;
    chunk->next = NULL;
    chunk->prev = NULL;
    chunk->ref_cnt = 1;
    chunk->chpool = chpool;
    chunk->data = mmap(NULL, get_chunk_size(len), chpool->prot, MAP_SHARED,
                       chpool->fd, get_chunk_size(1));

    if(chunk->data == MAP_FAILED)
        return errno;

    return 0;
}



static int chunk_destruct(chunk_t *chunk)
{
    if(munmap(chunk->data, chunk->len * get_chunk_size(1)) == -1)
        return errno;

    free(chunk);
    chunk = NULL;

    return 0;
}



static int chunk_construct(chunk_t *chunk, chpool_t *chpool,
                           off_t index, off_t len)
{
    chunk = (chunk_t *)calloc(1, sizeof(chunk_t));
    if(chunk == NULL) return ENOMEM;

    int error = chunk_init(chunk, chpool, index, len);
    if(error)
        return error;

    error = chpool_add(chunk);
    if(error)
        return error;

    return 0;
}



static int chunk_get(chunk_t *chunk, chpool_t *chpool,
                     off_t index, off_t len)
{
    int error = find_value(chpool->ht, (const hash_key_t)index, (value_t *)chunk);
    if(error)
        return error;

    if(chunk->len < len)
        return ENOKEY;

    return 0;
}



int chunk_acquire(chunk_t *chunk, chpool_t *chpool,
                  off_t offset, size_t size)
{
    if(!chpool || !chunk || !chpool->ht ||
       !chpool->fd < 0 || chpool->nr_pages > chpool->size)
        return EINVAL;

    if(size == 0)
    {
        chunk = NULL;
        return 0;
    }

    off_t index = offset / get_chunk_size(1);
    off_t len = size / get_chunk_size(1) + 1;

    int error = chunk_get(chunk, chpool, index, len);
    if(error == ENOKEY)
    {
        error = chpool_del(chunk);
        if(error && error != EBUSY)
            return error;

        chunk_construct(chunk, chpool, index, len);

        return 0;
    }
    else
        return 0;
}



int chunk_find(chunk_t *chunk, chpool_t * chpool,
               size_t size, off_t offset)
{
    if(!chunk || !chpool || !chpool->ht ||
        chpool->fd < 0 || chpool->nr_pages > chpool->size)
        return EINVAL;

    if(size == 0)
    {
        chunk = NULL;
        return 0;
    }

    off_t index = offset / get_chunk_size(1);
    off_t len = size / get_chunk_size(1) + 1;

    return chunk_get(chunk, chpool, index, len);
}


int chunk_release(chunk_t *chunk)
{
    if(!chunk)
        return EINVAL;

    if(chunk->ref_cnt == 0)
        return EAGAIN;

    if(chunk->ref_cnt == 1)
    {
        if(chunk->prev)
            chunk->prev->next = chunk->next;

        if(chunk->next)
            chunk->next->prev = chunk->prev;

        chunk->next = chunk->chpool->chunk_list;
        chunk->prev = NULL;
    }

    return 0;
}



int chunk_get_mem(chunk_t *chunk, off_t offset, void *buf)
{
    if(!chunk || !buf)
        return EINVAL;

    off_t left = get_chunk_size(chunk->index);
    off_t right = left + get_chunk_size(chunk->len);

    if(offset < left || offset > right)
        return EINVAL;

    buf = chunk->data + (offset - left);

    return 0;
}



static size_t get_chunk_size(off_t multiplier)
{
    return multiplier * sysconf(_SC_PAGESIZE);
}
//*******************************************************************
//*******************************************************************
//*******************************************************************

static int chpool_init(chpool_t *chpool, size_t size, int fd, int prot)
{
    chpool->chunk_list = NULL;
    chpool->fd = fd;
    chpool->size = size;
    chpool->nr_pages = 0;
    chpool->prot = prot;

    chpool->ht = hash_table_init(DEFAULT_HASHTABLE_SIZE, hash_fnv1a);
    if(chpool->ht == NULL)
        return ENOMEM;

    chpool->mem_ht = hash_table_init(DEFAULT_HASHTABLE_SIZE, hash_fnv1a);
    if(chpool->mem_ht == NULL)
        return ENOMEM;

    return 0;
}



int chpool_construct(chpool_t *chpool, size_t mem_limit, int fd, int prot)
{
    size_t size = mem_limit ? mem_limit / get_chunk_size(1) : DEFAULT_CHPOOL_SIZE;

    chpool = (chpool_t *)calloc(1, sizeof(chpool_t));

    return chpool_init(chpool, size, fd, prot);
}



int chpool_destruct(chpool_t *chpool)
{
    if(!chpool)
        return EINVAL;

    while(chpool->chunk_list)
    {
        chpool->chunk_list = chpool->chunk_list->next;

        int error = chunk_destruct(chpool->chunk_list->prev);
        if(error)
            return error;
    }

    chpool->nr_pages = 0;

    int error = hash_table_deinit(chpool->ht);
    if(error)
        return error;

    error = hash_table_deinit(chpool->mem_ht);
    if(error)
        return error;

    error = close(chpool->fd);
    if(error == -1)
        return errno;

    free(chpool);

    return 0;
}



static int chpool_del(chunk_t *chunk)
{
    chpool_t *chpool = chunk->chpool;

    if(chunk->prev)
        chunk->prev->next = chunk->next;

    if(chunk->next)
        chunk->next->prev = chunk->prev;

    for(unsigned int i = 0; i < chunk->len; i++)
    {
        int error = delete_item(chpool->ht, chunk->index + i);
        if(error && error != ENOKEY)
            return error;
    }

    chpool->nr_pages -= chunk->len;

    int error = chunk_destruct(chunk);
    if(error)
        return error;

    return 0;
}


static int chpool_add(chunk_t *chunk)
{
    chpool_t *chkpool = chunk->chpool;

    int error = 0;
    while(chkpool->nr_pages + chunk->len > chkpool->size && !(error = chpool_del(chkpool->chunk_list)));

    if(error == EBUSY)
        return ENOBUFS;

    if(error)
        return error;

    chunk->next = chkpool->chunk_list;
    chunk->prev = NULL;
    chkpool->chunk_list = chunk;

    for(unsigned int i = 0; i < chunk->len; i++)
    {
        error = add_item(chkpool->ht, chunk->index + i, (value_t)chunk);
        if(error)
            return error;
    }

    chkpool->nr_pages += chunk->len;

    return 0;
}




int chpool_fd(chpool_t *chpool)
{
    if(!chpool)
        return EINVAL;

    return chpool->fd;
}



int chpool_mem_add(void *ptr, chunk_t *chunk)
{
    if(!ptr || !chunk)
        return EINVAL;

    int error = add_item(chunk->chpool->mem_ht, (hash_key_t)ptr, (value_t)chunk);
    if(error)
        return error;

    return 0;
}



int chpool_mem_get(chpool_t *chpool, void *ptr, chunk_t *chunk)
{
    return find_value(chpool->mem_ht, (hash_key_t)ptr, (value_t)chunk);
}
