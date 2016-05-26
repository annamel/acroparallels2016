#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "chunk_manager.h"

struct Pool* init_pool(){

    size_t spool_size = 64;

    if(sizeof(void*) == 4) {
        spool_size = 128;
    }

    size_t size = spool_size * sizeof(struct Chunk) + sizeof(struct Spool) + sizeof(struct Pool);
    struct Pool* pool = (struct Pool*)calloc(1, size);
    if(!pool)
        return 0;


    pool -> data = (struct Spool**)calloc(sizeof(void*), INIT_SPOOL_COUNT);
    if(!pool -> data) {
        free(pool);
        return 0;
    }

    pool -> spool_size = spool_size;
    pool -> data[pool -> spool_count++] = (struct Spool*)((void*)pool + sizeof(struct Pool));
    pool -> spool_max = INIT_SPOOL_COUNT;

    return pool;
}

ssize_t destruct_pool(struct Pool* pool) {
    if(!pool)
        return 0;

    for(int i = 1; i < pool -> spool_count; i++)
        free(pool -> data[i]);

    free(pool);

    return 0;
}

struct Chunk* allocate_chunk(struct Pool* pool, void* ptr, off_t offset, size_t size){
    if(!pool)
        return 0;

    for(int i = 0; i < pool -> spool_count; i++) {

        if(pool -> data[i] -> count == pool -> spool_size)
            continue;

        struct Chunk* tmp = pool -> data[i] -> data;

        for(int j = 0; j < pool -> spool_size; j++) {
            if(tmp[j].state == EMPTYCM) {
                tmp[j].state = READYCM;
                tmp[j].refcount++;

                tmp[j].ptr = ptr;
                tmp[j].offset = offset;
                tmp[j].size = size;

                pool -> data[i] -> count++;


                return &tmp[j];
            }

        }
    }

    //actions if all spools are full

    void* free_candidate = 0;

    if(pool -> spool_count == pool -> spool_max) {

        struct Spool** tmp_data = (struct Spool**)calloc(pool->spool_max * 2, sizeof(void*));
        if(!tmp_data)
            return 0;
        memcpy(tmp_data, pool -> data, sizeof(void*) * pool -> spool_count);

        free_candidate = pool -> data;
        pool -> data = tmp_data;
        pool -> spool_max *= 2;
    }

    size_t index = pool -> spool_count;
    struct Spool* tmp = (struct Spool*)calloc(1, sizeof(struct Spool) + sizeof(struct Chunk) * pool -> spool_size);
    if(!tmp)
        return 0;

    pool -> data[index] = tmp;
    pool -> spool_count++;

    free(free_candidate); //Actually this can be a real problem if someone is handling previous pool -> data

    return allocate_chunk(pool, ptr, offset, size);
}

ssize_t deref_chunk(struct Pool* pool, struct Chunk* item) {
    if(!pool || !item)
        return -1;

    if(!item -> refcount) {
        item -> refcount--;
        return item -> refcount;
    }

    for(int i = 0; i < pool -> spool_count; i++) {
        size_t bottom = (size_t)item - (size_t)(pool -> data[i] -> data);
        size_t top = (size_t)(pool -> data[i] -> data + pool -> spool_size) - (size_t)item;

        if(bottom <= pool -> spool_size * sizeof(struct Chunk) && top <= pool -> spool_size * sizeof(struct Chunk)) {
            item -> state = BUSYCM;
            item -> refcount--;

            pool -> data[i] -> count--;
            item -> state = EMPTYCM;

            return 0;
        }
    }

    return -1;
}
