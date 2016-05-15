#include "../hash_table/hash_table.h"
#include "../List/list.h"
#include "chunk_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>



chunk_t* chunk_init(off_t index, off_t len, chunk_pool_t* cpool){
    //log_write(DEBUG, "chunk_init: start of work);
    if(!cpool) {
        //log_write(ERROR, "chunk_init: invalid args (cpool));
        return NULL;
    }
    
    // chunk_t* chunk = (chunk_t*)calloc(1, sizeof(chunk_t));
    
    chunk_t* chunk;
    if(cpool->free_chunks->size == 0) {
        if(cpool->zero_chunks->size == 0) {
            cpool->loafs = (chunk_t**)realloc(cpool->loafs, (cpool->loafs_count+1)*sizeof(chunk_t*));
            if(!cpool->loafs) {
                //log_write(ERROR, "chunk_init: cannot reallocate memory);
                return NULL;
            }
            
            cpool->loafs[cpool->loafs_count] = (chunk_t*)calloc(DEFAULT_ARRAY_SIZE, sizeof(chunk_t));
            if(!cpool->loafs[cpool->loafs_count]) {
                //log_write(ERROR, "chunk_init: cannot allocate memory);
                return NULL;
            }
            cpool->loafs_count += 1;
            
            int i = 0;
            for(i; i < DEFAULT_ARRAY_SIZE; i++) {
                list_add_last(cpool->free_chunks, (value_t*)&(cpool->loafs[cpool->loafs_count - 1])[i]);
            }
            
            chunk = cpool->free_chunks->head->value;
            list_delete_first(cpool->free_chunks);
        } else {
            chunk = cpool->zero_chunks->head->value;
            if(munmap(chunk->data, chunk->len) == -1) {
                //log_write(ERROR, "chunk_init: cannot munmap memory);
                return errno;
            }
            list_delete_first(cpool->zero_chunks);
        }
    } else {
        chunk = cpool->free_chunks->head->value;
        list_delete_first(cpool->free_chunks);
    }
    
    if(!chunk) {
        //log_write(ERROR, "chunk_init: cannot allocate memory);
        return NULL;
    }
    
    chunk->data = mmap(NULL, cpool->pg_size*(len), cpool->protection, MAP_SHARED,
                       cpool->fd, cpool->pg_size*(index));
    
    if(chunk->data == MAP_FAILED) {
        return NULL;
    }
    
    chunk->index = index;
    chunk->len = len;
    chunk->ref_counter = 1;
    chunk->cpool = cpool;
    
    int error = ht_add_item(cpool->hash, (hkey_t)chunk->index, (hvalue_t)chunk);
    if(error) {
        //log_write(ERROR, "chunk_init: cannot add chunk to hash table);
        return error;
    }
    
    //log_write(DEBUG, "chunk_init: end of work);
    return chunk;
}

chunk_pool_t* chunk_pool_init(int fd, int prot) {
    //log_write(DEBUG, "chunk_pool_init: start of work);
    if(fd < 0 || (!(prot & PROT_READ) && !(prot & PROT_WRITE))){
        //log_write(ERROR, "chunk_pool_init: invalid args);
        return NULL;
    }
    
    chunk_pool_t* cpool = (chunk_pool_t*)calloc(1, sizeof(chunk_pool_t));
    if(!cpool) {
        //log_write(ERROR, "chunk_pool_init: cannot allocate memory);
        return NULL;
    }
    
    cpool->fd = fd;
    cpool->protection = prot;
    cpool->zero_chunks = list_init();
    cpool->free_chunks = list_init();
    cpool->hash = ht_init(DEFAULT_HASH_TABLE_SIZE, hash_fnv1a);
    cpool->loafs = (chunk_t**)calloc(1, sizeof(chunk_t*));
    *cpool->loafs = (chunk_t*)calloc(DEFAULT_ARRAY_SIZE, sizeof(chunk_t));
    cpool->loafs_count = 1;
    cpool->is_mapped = 0;
    cpool->pg_size = sysconf(_SC_PAGESIZE);

    struct stat sb = {0};
    int err = fstat(fd, &sb);
    cpool->file_size = sb.st_size;

    
    int i = 0;
    for(i; i < DEFAULT_ARRAY_SIZE; i++) {
        list_add_last(cpool->free_chunks, (value_t*)&(*cpool->loafs)[i]);
    }
    
    if(!cpool->hash || !cpool->zero_chunks || !cpool->free_chunks) {
        //log_write(ERROR, "chunk_pool_init: cannot allocate memory);
        return NULL;
    }
    
    return cpool;
}

int chunk_pool_find(chunk_pool_t* cpool, chunk_t** chunk, off_t index, off_t len) {
    if(!cpool) {
        //log_write(ERROR, "chunk_pool_hashtable_find: cannot allocate memory);
        return EINVAL;
    }
    
    hkey_t key = (hkey_t)index;
    
    int idx = cpool->hash->hash_func(key, HASH_CONST_2) % cpool->hash->size;
    item_t* item_ptr = cpool->hash->table[idx];
    if(!item_ptr)
        return ENOKEY;

    while(item_ptr) {
        if(item_ptr->key <= key && ((chunk_t*)item_ptr->value)->len >= len + (key - item_ptr->key)) {
            *chunk = (chunk_t*)item_ptr->value;
            return 0;
        } else {
            item_ptr = item_ptr->next;
            continue;
        }
    }
    
    return ENOKEY;
}

int chunk_pool_deinit(chunk_pool_t *cpool) {
    if(!cpool) {
        return EINVAL;
    }
    
    int err = list_deinit(cpool->free_chunks);
    if(err) return err;
    
    err = list_deinit(cpool->zero_chunks);
    if(err) return err;
    
    err = ht_deinit(cpool->hash);
    
    int i = 0; int j = 0;
    for(i; i < cpool->loafs_count; i++) {
        for(j; j < DEFAULT_ARRAY_SIZE; j++) {
            chunk_deinit(&(cpool->loafs)[i][j]);
        }
        free((cpool->loafs)[i]);
    }
    free(cpool->loafs);
    
    err = close(cpool->fd);
    if(err == -1) return errno;
    free(cpool);
    
    return 0;
}

int chunk_deinit(chunk_t *chunk) {
    
}

int get_chunk_size(off_t len) {
    return len*sysconf(_SC_PAGESIZE);
}

int chunk_release(chunk_t* chunk) {
    if(chunk == NULL) {
        return EINVAL;
    }
    
    int err = munmap(chunk->data, chunk->cpool->pg_size*(chunk->len));
    if(err == -1) return errno;
    
    err = ht_del_item_by_kav(chunk->cpool->hash, (hkey_t)chunk->index, (hvalue_t)chunk);
    if(err) return err;
    
    chunk->ref_counter = 0;
    chunk->index = 0;
    chunk->len = 0;
    chunk->data = NULL;
    
    
    err = list_add_last(chunk->cpool->free_chunks, (value_t)chunk);
    if(err) return err;
    
    return 0;
    
}

