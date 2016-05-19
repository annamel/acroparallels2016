//
// Created by voddan on 19/05/16.
//

#include <sorted_list.h>
#include <stddef.h>
#include <stdlib.h>
#include "chunk_manager.h"

typedef struct chman_slice {
    int             ref_cnt;
    void*           ptr;

    struct slice*   next;       // NULL
    int             next_num;   // 0
} slice;


typedef struct chman_page {
    chman_index    index;
    sorted_list*    slices;     // max
} page;

typedef struct chunk_manager{
    file_info*  info;
    off_t       page_size;
} manager;


// PRIVATE
void* load_pages(manager* hp, chman_index first, int num) {
    return mmap(NULL, (size_t) (num * hp->page_size),
                PROT_WRITE, MAP_SHARED,
                hp->info->file_descriptor, first * hp->page_size);
}


chunk_manager* chman_alloc(file_info * info, off_t page_size) {
    manager* m = malloc(sizeof(manager));
    m->info = info;
    m->page_size = page_size;

    return m;
}

void chman_free(chunk_manager* chm) {
    free(chm);
}


void* chman_slice_ptr(chman_slice* s) {
    return s->ptr;
}


chman_slice* chman_up_range(chunk_manager* chm, chman_index first, int num) {
    void * ptr = load_pages(chm, first, num);

    slice* prev = NULL;
    for (int i = 0; i < num; ++i) {
        slice* slice = malloc(sizeof(slice));
        slice->ptr = ptr;
        slice->ref_cnt = 1; //TEST
        slice->next_num = num - i - 1;
        slice->next = NULL;
        if(prev) {
            prev->next = slice;
        }
    }
}

void chman_up_bunch(chunk_manager * chm, chman_index first, int num, chman_slice ** arr) {
    slice* slice = chman_up_range(chm, first, num); // todo

    arr[0] = slice;
    for (int i = 0; i < num; ++i) {
        slice = slice->next;
        arr[i] = slice;
    }
}

chman_slice* chman_down(chunk_manager* chm, chman_slice* sl) {
    slice* next = sl->next;

    sl->ref_cnt -= 1;
    //todo
    return next;
}