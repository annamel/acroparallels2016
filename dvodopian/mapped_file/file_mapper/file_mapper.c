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

#include <key_set.h>
#include <sorted_list.h>

/*
 * a file is separated into pages
 * a chman_page may be mapped onto a slice
 * several slices in a row are a chunk
 * */

/*
* STRUCTURES
* */


//typedef struct slice {
//    int             ref_cnt;
//    void*           ptr;
//
//    struct slice*   next;       // NULL
//    int             next_num;   // 0
//} slice;
//
//
//typedef struct {
//    unsigned int    index;
//    sorted_list*    slices;     // max
//} chman_page;


/*
 * HANDLES
 * */


typedef struct {
    long            sys_page_size;
    size_t          sys_max_memory;

    char const *    file_name;
    int             file_descriptor;
    off_t           file_size;

    int             page_num;

    key_set*        loaded_pages;  // index -> chman_page*
} handle;


typedef struct {
    handle * file;
    size_t size;
    off_t offset;

    void * ptr;

    slice* slice_begin;
    int slice_num;
} map_handle;



/*
 * PAGE LOADING
 * */

//off_t page_offset(handle const * hp, off_t offset) {
//    assert(offset < hp->file_size);
//    return offset % hp->sys_page_size;
//}
//
//unsigned int page_index(handle const * hp, off_t offset) {
//    assert(offset < hp->file_size);
//    return (unsigned int) (offset / hp->sys_page_size);
//}
//
//void* load_pages(handle const * hp, unsigned int pi_begin, unsigned int pi_end) {
//    assert(pi_begin < pi_end);
//
//    off_t offset = pi_begin * hp->sys_page_size;
//    off_t size = (pi_end - pi_begin + 1) * hp->sys_page_size;
//
//    void * ptr = mmap(NULL, (size_t) size, PROT_WRITE, MAP_SHARED, hp->file_descriptor, offset);
//
//    return ptr;
//}
//
//chman_page* page_alloc(handle* hp, unsigned int index) {
//    chman_page* p = malloc(sizeof(chman_page));
//    p->index = index;
//
//    int comparator(void * item) {
//        return -((slice*) item)->next_num;
//    }
//
//    p->slices = list_alloc(comparator);
//}
//
//void page_free(handle* hp, chman_page* p) {
//    list_free(p->slices);
//    free(p);  // todo: unmap and free slices
//}

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
    hp->page_num = page_index(hp, hp->file_size) + 1;

    hp->loaded_pages = set_alloc();

    *mf = hp;

    return 0;
}

int mf_close(mf_handle_t mf) {
    handle* hp = (handle*) mf;

    close(hp->file_descriptor);

    for (set_key_t i = 0; i < hp->page_num; ++i) {
        page* p = set_get(hp->loaded_pages, i);
        
        if(p) {
            page_free(mf, p);
        }
    }

    set_free(hp->loaded_pages); 
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
    
    unsigned int begin = page_index(hp, offset);
    unsigned int anEnd = page_index(hp, (off_t) (offset + size));

    page* p = set_get(hp->loaded_pages, begin);
    if(p) {
        slice* s = list_get(p->slices, 0);
        if(s && s->next_num >= anEnd - begin) {
            mhp->ptr = s->ptr + page_offset(hp, offset);

            slice * sp = s;
            for (int i = 0; i < anEnd - begin + 1; ++i) {
                sp->ref_cnt += 1;
                sp = sp->next;
            }
        } else {
            void * arr = load_pages(hp, begin, anEnd);

            slice* prev_sp = NULL;
            for (int i = anEnd - begin; i >= 0; i--) {

                slice * sp = malloc(sizeof(slice));
                sp->ref_cnt = 1;
                sp->next_num = anEnd - begin - i;
                sp->next = prev_sp;
                sp->ptr = arr + i * hp->sys_page_size;

                list_add(p->slices, sp);

                prev_sp = sp;
            }

            mhp->ptr = arr + page_offset(hp, offset);
        }
    } else {
        //set_set(hp->loaded_pages)
    }
    mhp->ptr = load_pages(hp, begin, anEnd);


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















