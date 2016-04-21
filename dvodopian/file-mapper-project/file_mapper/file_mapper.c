//
// Created by voddan on 20/04/16.
//


#include <stdatomic.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>
#include "file_mapper.h"
#include "handles.h"

/*
 * PAGE LOADING API
 * */

long get_page_index(handle const * hp, off_t offset) {
    assert(hp->file_size < offset);
    return offset / hp->sys_page_size;
}

long get_page_num(handle const * hp, off_t offset, off_t size) {
    long first = get_page_index(hp, offset);
    long last = get_page_index(hp, offset + size);

    return last - first + 1;
}

void* page_load_mult(handle const * hp, long page_index, int page_num) {
    assert(page_index > 0);
    assert(page_num > 0);

    off_t offset = page_index * hp->sys_page_size;
    off_t size = page_num * hp->sys_page_size;

    void * ptr = mmap(NULL, (size_t) size, PROT_WRITE, MAP_SHARED, hp->file_descriptor, offset);

    return ptr;
}

void* page_load(handle const * hp, long page_index) {
    return page_load_mult(hp, page_index, 1);
}

/*
 * PUBLIC API
 * */

int mf_open(char const * name, size_t max_memory, mf_handle_t * mf) {
    handle* hp = malloc(sizeof(handle));
    hp->sys_page_size = sysconf(_SC_PAGE_SIZE);
    hp->sys_max_memory = max_memory;

    hp->file_name = name;
    hp->file_descriptor = open(name, O_RDWR);

    struct stat file_stat;
    fstat(hp->file_descriptor, &file_stat);
    hp->file_size = file_stat.st_size;

    *mf = hp;

    return 0;
}

int mf_close(mf_handle_t mf) {
    handle* hp = (handle*) mf;

    close(hp->file_descriptor);

    free(mf);
    return 0;
}

int mf_read(mf_handle_t mf, off_t offset, size_t size, void * buf) {
    return 0;
}

int mf_write(mf_handle_t mf, off_t offset, size_t size, void * buf) {
    return 0;
}

int mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t * mm, void ** ptr) {
    handle* hp = (handle*) mf;

    map_handle* mhp = malloc(sizeof(map_handle));
    mhp->file = hp;
    mhp->offset = offset;
    mhp->size = size;

    mhp->ptr  = mmap(NULL, size, PROT_WRITE, MAP_SHARED, hp->file_descriptor, offset);

    *ptr = mhp->ptr;
    *mm = mhp;

    return 0;
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mm) {
    map_handle* mhp = (map_handle*) mm;

    munmap(mhp->ptr, mhp->size);

    free(mhp);
    return 0;
}

int mf_file_size(mf_handle_t mf, size_t * size) {
    *size = (size_t) ((handle*) mf)->file_size;
    return 0;
}

int mf_file_sync(mf_handle_t * mf) {
    return 0;
}















