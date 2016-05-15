#include "common_types.h"
#include "logger.h"
#include <errno.h>
#include <sys/mman.h>
#include <malloc.h>
#include <unistd.h>
#include <stdbool.h>


size_t get_chunk_size(off_t multiplier) {
    return multiplier * sysconf(_SC_PAGESIZE);
}

int ch_init(off_t index, off_t length, ch_pool_t *ch_pool) {
    if(ch_pool == NULL) {
        write_log_to_file(Error, "ch_init: invaid input!\n");
        return -1;
    }

    size_t chunk_size_min = ch_pool -> chunk_size_min;
    chunk_t *buf;
    list_t *list_of_free_chunks = ch_pool -> list_of_free_chunks;
    list_t *list_zero_ref_count = ch_pool -> list_zero_ref_count;

    if(list_of_free_chunks -> size == 0) {
        if(list_zero_ref_count -> size == 0) {
            ch_pool -> arrays_cnt += 1;

            size_t arrays_cnt = ch_pool -> arrays_cnt;

            ch_pool -> pool = (chunk_t**)realloc(ch_pool->pool, (arrays_cnt) * sizeof(chunk_t*));
            if(ch_pool == NULL) {
                write_log_to_file(Error, "ch_init: problem with realloc in ch_pool init!\n");
                return -2;
            }

            chunk_t **pool = ch_pool -> pool;
            pool[arrays_cnt -1] = (chunk_t*)calloc(DEFAULT_ARRAY_SIZE, sizeof(chunk_t));
            if(pool[arrays_cnt -1] == NULL) {
                write_log_to_file(Error, "ch_init: problem with calloc in pool init!\n");
            }

            int i = 0;
            for(i = 0; i < DEFAULT_ARRAY_SIZE; i++) {
                list_add_last(list_of_free_chunks, &((pool)[arrays_cnt - 1][i]));
                (pool)[arrays_cnt - 1][i].data = NULL;
                (pool)[arrays_cnt - 1][i].ref_counter = 0;
                (pool)[arrays_cnt - 1][i].ch_pool = ch_pool;
                (pool)[arrays_cnt - 1][i].index = 0;
                (pool)[arrays_cnt - 1][i].length = 0;
            }

            buf = list_of_free_chunks -> first -> data;
            buf -> ref_counter += 1;
            list_erase_first(list_of_free_chunks);
        } else {
            buf = list_zero_ref_count -> first -> data;
            buf -> ref_counter += 1;
            list_erase_first(list_zero_ref_count);
            if(munmap(buf -> data, chunk_size_min * buf -> length) == -1) {
                write_log_to_file(Error, "ch_init: problem with munmap occured!\n");
                return -3;
            }
        }
    } else {

        buf = list_of_free_chunks -> first -> data;
        buf -> ref_counter += 1;
        list_erase_first(list_of_free_chunks);
    }

    buf -> data = mmap(NULL, chunk_size_min * length, ch_pool -> prot, MAP_SHARED, ch_pool -> fd, chunk_size_min * index);
    if(buf -> data == MAP_FAILED) {
        write_log_to_file(Error, "ch_init: mmap failed, errno=%d!\n");
        return -4;
    }

    buf -> length = length;
    buf -> index = index;
    buf -> chunk_size_min = chunk_size_min;
    ch_pool -> last_chunk = buf;

    int res_code = add_element(buf);
    if(res_code) {
        write_log_to_file(Error, "ch_init: add_element function returned error value!\n");
        return -5;
    }

    return 0;
}

int chunk_release(chunk_t* chunk) {

    if(chunk == NULL) {
        write_log_to_file(Error, "chunk_release: invalid input!\n");
        return EINVAL;
    }

    int res_code = munmap(chunk -> data, (chunk -> chunk_size_min) * (chunk -> length));
    if(res_code == -1) {
        write_log_to_file(Error, "chunk_release: problems with munmap!\n");
        return errno;
    }

    res_code = remove_element(chunk -> ch_pool -> h_table, chunk -> index, chunk -> length);
    if(res_code) {
        write_log_to_file(Error, "chunk_release: remove_element returned error code!\n");
        return res_code;
    }

    res_code = list_add_last(chunk -> ch_pool -> list_of_free_chunks, chunk);
    if(res_code) {
        write_log_to_file(Error, "chunk_release: list_add_last returned error code!\n");
        return res_code;
    }

    chunk -> ref_counter = 0;
    chunk -> index = 0;
    chunk -> length = 0;
    chunk -> data = NULL;

    return 0;
}

int chunk_clear(chunk_t* chunk) {

    if(chunk == NULL) {
        write_log_to_file(Error, "chunk_clear: invalid input!\n");
        return EINVAL;
    }

    if(chunk -> data != NULL) {
        int res_code = munmap(chunk -> data, (chunk -> chunk_size_min) * (chunk -> length));
        if(res_code == -1) {
            write_log_to_file(Error, "chunk_clear: munmap returned error code!\n");
            return errno;
        }
    }

    chunk -> ch_pool = NULL;

    return 0;
}

int ch_find(ch_pool_t *ch_pool, off_t index, off_t length) {
    return find_value(ch_pool -> h_table, index, length);
}

ch_pool_t *ch_pool_init(int fd, int prot) {

    if(fd < 0 || (!(prot & PROT_READ) && !(prot & PROT_WRITE))) {
        write_log_to_file(Error, "ch_pool_init: invalid input!\n");
        return NULL;
    }

    ch_pool_t *buf = (ch_pool_t*)calloc(1, sizeof(ch_pool_t));
    if(buf == NULL) {
        write_log_to_file(Error, "ch_pool_init: problems with buf init!\n");
        return NULL;
    }

    buf -> fd = fd;
    buf -> prot = prot;
    buf -> fdd = 0;

    buf -> list_of_free_chunks = list_init();
    if(buf -> list_of_free_chunks == NULL) {
        write_log_to_file (Error, "ch_pool_init: list of free chunks init failed!\n");
        return NULL;
    }

    buf -> list_zero_ref_count = list_init();
    if(buf -> list_zero_ref_count == NULL) {
        write_log_to_file (Error, "ch_pool_init: list of chunks with zero reference counter init failed!\n");
        return NULL;
    }

    buf -> h_table = hash_table_init(DEFAULT_HASH_TABLE_SIZE);
    if(buf -> h_table == NULL) {
        write_log_to_file (Error, "ch_pool_init: hash table initialization failed!\n");
        return NULL;
    }

    sem_init((&(buf->lock)), 0, 1);

    buf -> pool = (chunk_t**)calloc(1, sizeof(chunk_t*));
    *(buf -> pool) = (chunk_t*)calloc(DEFAULT_ARRAY_SIZE, sizeof(chunk_t));
    if((buf -> pool == NULL) || (*(buf -> pool) == NULL)) {
        write_log_to_file (Error, "ch_pool_init: pool init failed!\n");
        return NULL;
    }

    buf -> arrays_cnt = 1;

    int i = 0;
    for(i = 0; i < DEFAULT_ARRAY_SIZE; i++) {
        if(list_add_last(buf -> list_of_free_chunks, (data_t)(&((*(buf -> pool))[i])))) {
            write_log_to_file (Error, "ch_pool_init: free chunk can't be added to appropriate list!\n");
            return NULL;
        }

        chunk_t **pool = buf -> pool;
        (*pool)[i].data = NULL;
        (*pool)[i].ref_counter = 0;
        (*pool)[i].ch_pool = buf;
        (*pool)[i].index = 0;
        (*pool)[i].length = 0;
    }

    return buf;
}

int ch_pool_deinit(ch_pool_t* ch_pool) {

    if(ch_pool == NULL) {
        write_log_to_file (Error, "ch_pool_deinit: invalid input!\n");
        return EINVAL;
    }

    int res_code = list_deinit(ch_pool -> list_of_free_chunks);
    if(res_code) {
        write_log_to_file (Error, "ch_pool_deinit: deinit of list of free chunks failed!\n");
        return res_code;
    }

    res_code = list_deinit(ch_pool -> list_zero_ref_count);
    if(res_code) {
        write_log_to_file (Error, "ch_pool_deinit: deinit of list of chunks with reference counter = 0 failed!\n");
        return res_code;
    }

    res_code = hash_table_deinit(ch_pool -> h_table);
    int i = 0;
    int j = 0;
    for(i = 0; i < ch_pool -> arrays_cnt; i++) {
        for(j = 0; j < DEFAULT_ARRAY_SIZE; j++) {
            chunk_clear(&((ch_pool -> pool)[i][j]));
        }
        free((ch_pool -> pool)[i]);
    }
    free(ch_pool -> pool);
    
    res_code = close(ch_pool -> fd);
    if(res_code == -1) {
        write_log_to_file (Error, "ch_pool_deinit: func close retruned -1!\n");
        return errno;
    }

    free(ch_pool);

    return 0;
}


