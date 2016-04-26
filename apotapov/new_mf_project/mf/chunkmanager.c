#include "chunkmanager.h"
#include "logger.h"
#include "hash_table.h"

#include <errno.h>
#include <sys/mman.h>
#include <malloc.h>
#include <unistd.h>
#include <stdbool.h>
#include "logger.h"


size_t get_chunk_size(off_t multiplier) {
    return multiplier * sysconf(_SC_PAGESIZE);
}

int ch_init(off_t index, off_t length, ch_pool_t *ch_pool) {
    if(ch_pool == NULL) {
        write_log(Error, "ch_init (index=%d, len=%d): invaid input!\n", index, length);
        return -1;
    }
    write_log(Info, "ch_init (index=%d, len=%d): started!\n", index, length);
    chunk_t *buf;
    if(ch_pool -> list_of_free_chunks -> size == 0) {
        write_log(Info, "ch_init (index=%d, len=%d): list of free chunks is empty!\n", index, length);
        if(ch_pool -> list_zero_ref_counters -> size == 0) {
            write_log(Info, "ch_init (index=%d, len=%d): list of chunks with 0 reference counter is empty!\n", index, length);
            ch_pool -> arrays_cnt += 1;
            ch_pool -> pool = (chunk_t**)realloc(ch_pool->pool, (ch_pool -> arrays_cnt) * sizeof(chunk_t*));
            if(ch_pool == NULL) {
                write_log(Error, "ch_init (index=%d, len=%d): problem with realloc in ch_pool init!\n", index, length);
                return -1;
            }
            ch_pool -> pool[ch_pool -> arrays_cnt -1] = (chunk_t*)calloc(DEFAULT_ARRAY_SIZE, sizeof(chunk_t));
            if(ch_pool -> pool[ch_pool -> arrays_cnt -1] == NULL) {
                write_log(Error, "ch_init (index=%d, len=%d): problem with calloc in pool init!\n", index, length);
            }
            int i = 0;
            for(i = 0; i < DEFAULT_ARRAY_SIZE; i++) {
                  list_add_last(ch_pool -> list_of_free_chunks, &((ch_pool -> pool)[arrays_cnt - 1][i]));
                  (ch_pool -> pool)[arrays_cnt - 1][i].data = NULL;
                  (ch_pool -> pool)[arrays_cnt - 1][i].ref_counter = 0;
                  (ch_pool -> pool)[arrays_cnt - 1][i].ch_pool = ch_pool;
                  (ch_pool -> pool)[arrays_cnt - 1][i].index = 0;
                  (ch_pool -> pool)[arrays_cnt - 1][i].length = 0;
            }
            buf = ch_pool -> list_of_free_chunks -> first -> data;
            buf -> ref_counter += 1;
            list_erase_first(ch_pool -> list_of_free_chunks);
        } else {
            buf = ch_pool -> list_zero_ref_counters -> first -> data;
            buf -> ref_counter += 1;
            list_erase_first(ch_pool -> list_zero_ref_counters);
            if(munmap(buf -> data, get_chunk_size(buf -> length)) == -1) {
                write_log(Error, "ch_init (index=%d, len=%d): problem with munmap occured!\n", index, length);
                return -1;
            }
        }
    } else {
        buf = ch_pool -> list_of_free_chunks -> first -> data;
        buf -> ref_counter += 1;
        list_erase_first(ch_pool -> list_of_free_chunks);
    }
    buf -> data = mmap(NULL, get_chunk_size(length), ch_pool -> prot, MAP_SHARED, ch_pool -> fd, get_chunk_size(index));
    if(buf -> data == MAP_FAILED) {
        write_log(Error, "ch_init (index=%d, len=%d): mmap failed, errno=%d!\n", index, length, errno);
        return -1;
    }
    buf -> length = length;
    buf -> index = index;
    int res_code = add_element(buf);
    if(res_code) {
        write_log(Error, "ch_init (index=%d, len=%d): add_element function returned error value!\n", index, length);
        return -1;
    }
    write_log(Info, "ch_init (index=%d, len=%d): finished!\n", index, length);
    return 0;
}

int chunk_release(chunk_t* chunk) {
     write_log(Info, "chunk_release (index=%d, len=%d): started!\n", chunk -> index, chunk -> length);
    if(chunk == NULL) {
         write_log(Error, "chunk_release (index=%d, len=%d): invalid input!\n", chunk -> index, chunk -> length);
        return EINVAL;
    }
    int res_code = munmap(chunk -> data, get_chunk_size(chunk -> length));
    if(res_code == -1) {
        write_log(Error, "chunk_release (index=%d, len=%d): problems with munmap!\n", chunk -> index, chunk -> length);
        return errno;
    }
    res_code = remove_element(chunk -> ch_pool -> h_table, chunk -> index, chunk -> length);
    if(res_code) {
        write_log(Error, "chunk_release (index=%d, len=%d): remove_element returned error code!\n", chunk -> index, chunk -> length);
        return res_code;
    }
    res_code = list_add_last(chunk -> ch_pool -> list_of_free_chunks, chunk);
    if(res_code) {
        write_log(Error, "chunk_release (index=%d, len=%d): list_add_last returned error code!\n", chunk -> index, chunk -> length);
        return res_code;
    }
    chunk -> ref_counter = 0;
    chunk -> index = 0;
    chunk -> length = 0;
    chunk -> data = NULL;
    write_log(Info, "ch_release (index=%d, len=%d): finished!\n", chunk -> index, chunk -> length);
    return 0;
}

int chunk_clear(chunk_t* chunk) {
    write_log(Info, "chunk_clear: started!\n");
    if(chunk == NULL) {
        write_log(Error, "chunk_clear: invalid input!\n");
        return EINVAL;
    }
    if(chunk -> data != NULL) {
        int res_code = munmap(chunk -> data, get_chunk_size(chunk -> length));
        if(res_code == -1) {
            write_log(Error, "chunk_clear: munmap returned error code!\n");
            return errno;
        }
    }
    chunk -> ch_pool = NULL;
    write_log(Info, "chunk_clear: finished!\n");
    return 0;
}

//////////////////////////////////CHUNK_FUNCTIONS_END///////////////////////////////////////////////

int ch_find(ch_pool_t *ch_pool, off_t index, off_t length) {
    return find_value(ch_pool -> h_table, index, length); 
}


////////////////////////////////////ch_pool_functions_begin///////////////////////////////////


ch_pool_t *ch_pool_init(int fd, int prot) {
    if(fd < 0 || (!(prot & PROT_READ) && !(prot & PROT_WRITE))) {
        write_log(Error, "ch_pool_init (fd=%d, prot=%d): invalid input!\n", fd, prot);
        return NULL;
    }
    write_log(Info, "ch_pool_init (fd=%d, prot=%d): started!\n", fd, prot);
    ch_pool_t *buf = (ch_pool_t*)calloc(1, sizeof(ch_pool_t));
    if(buf == NULL) {
        write_log(Error, "ch_pool_init (fd=%d, prot=%d): problems with buf init!\n", fd, prot);
        return NULL;
    }
    buf -> fd = fd;
    buf -> prot = prot;
    buf -> list_of_free_chunks = list_init();
    if(buf -> list_of_free_chunks == NULL) {
        write_log (Error, "ch_pool_init (fd=%d, prot=%d): list of free chunks init failed!\n", fd, prot);
        return NULL;
    }

    buf -> list_zero_ref_counters = list_init();
    if(buf -> list_zero_ref_counters) {
        write_log (Error, "ch_pool_init (fd=%d, prot=%d): list of chunks with zero reference counter init failed!\n", fd, prot);
        return NULL;
    }
    buf -> h_table = hash_table_init(DEFAULT_HASH_TABLE_SIZE);
    if(buf -> h_table == NULL) {
        write_log (Error, "ch_pool_init (fd=%d, prot=%d): hash table initialization failed!\n", fd, prot);
        return NULL;
    }

    buf -> pool = (chunk_t**)calloc(1, sizeof(chunk_t*));
    *(buf -> pool) = (chunk_t*)calloc(DEFAULT_ARRAY_SIZE, sizeof(chunk_t));
    if((buf -> pool == NULL) || (*(buf -> pool) == NULL)) {
        write_log (Error, "ch_pool_init (fd=%d, prot=%d): pool init failed!\n", fd, prot);
        return NULL;
    }
    buf -> arrays_cnt = 1;
    int i = 0;
    for(i = 0; i < DEFAULT_ARRAY_SIZE; i++) {
        if(list_add_last(buf -> list_of_free_chunks, (data_t)(&((*(buf -> pool))[i])))) {
            write_log (Error, "ch_pool_init (fd=%d, prot=%d): free chunk can't be added to appropriate list!\n", fd, prot);
            return NULL;
        }
        (*(buf -> pool))[i].data = NULL;
        (*(buf -> pool))[i].ref_counter = 0;
        (*(buf -> pool))[i].ch_pool = buf;
        (*(buf -> pool))[i].index = 0;
        (*(buf -> pool))[i].length = 0;
    }
    write_log (Info, "ch_pool_init (fd=%d, prot=%d): finished!\n", fd, prot);
    return buf;
}

int ch_pool_deinit(ch_pool_t* ch_pool) {
    write_log (Info, "ch_pool_deinit: started!\n");
    if(ch_pool == NULL) {
        write_log (Error, "ch_pool_deinit: invalid input!\n");
        return EINVAL;
    }
    int res_code = list_deinit(ch_pool -> list_of_free_chunks);
    if(res_code) {
        write_log (Error, "ch_pool_deinit: deinit of list of free chunks failed!\n");
        return res_code;
    }
    res_code = list_deinit(ch_pool -> list_zero_ref_counters);
    if(res_code) {
        write_log (Error, "ch_pool_deinit: deinit of list of chunks with reference counter = 0 failed!\n");
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
        write_log (Error, "ch_pool_deinit: func close retruned -1!\n");
        return errno;
    }
    free(ch_pool);
    write_log (Info, "ch_pool_deinit: ffinished!\n");
    return 0;
}

///////////////////////////////////////ch_pool_functions_end////////////////////////////////////////////