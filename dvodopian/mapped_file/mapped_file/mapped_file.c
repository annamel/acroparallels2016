//
// Created by voddan on 19/05/16.
//

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <chunk_manager.h>
#include <memory.h>
#include "mapped_file.h"
#include "chunk_manager.h"

typedef struct {
    long            sys_page_size;

    char const *    file_name;
    int             file_descriptor;
    off_t           file_size;

    file_info*       info;

    int             page_num;

//    key_set*        loaded_pages;  // index -> chman_page*

    chunk_manager* manager;
} handle;


typedef struct {
    void * ptr;

    chman_slice* slice_first;
    int slice_num;
} map_handle;

/*
 * Returns NULL on failure.
 */
mf_handle_t mf_open(const char *pathname) {
    handle* hp = malloc(sizeof(handle));
    hp->sys_page_size = sysconf(_SC_PAGE_SIZE);

    hp->file_name = pathname;
    hp->file_descriptor = open(pathname, O_RDWR);

    struct stat file_stat;
    fstat(hp->file_descriptor, &file_stat);
    hp->file_size = file_stat.st_size;
    hp->page_num =  (int) hp->file_size / hp->sys_page_size;

    hp->info = malloc(sizeof(file_info));

    hp->info->file_size = hp->file_size;
    hp->info->file_descriptor = hp->file_descriptor;
    hp->info->file_name = hp->file_name;

    hp->manager = chman_alloc(hp->info, hp->sys_page_size);

    return hp;
}

/*
 * Returns 0 on success and -1 on failure
 */
int mf_close(mf_handle_t mf) {
    handle* hp = (handle*) mf;

    close(hp->file_descriptor);

    free(hp->info);
    chman_free(hp->manager);

    free(mf);
    return 0;
}

/*
 * Returns -1 on failure
 */
ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset) {

    handle* hp = (handle*) mf;
    off_t page_size =hp->sys_page_size;

    unsigned f =  offset / page_size;
    unsigned l = ( offset + count)/ page_size;

    int num = f - l + 1;

    chman_slice** arr = calloc(sizeof(chman_slice*), num);

    chman_up_bunch(hp->manager, f, num, arr);

//--------------
    size_t fps = offset % page_size;
    if(!fps) {
        fps = page_size;
    }
    memcpy(buf, chman_slice_ptr(arr[0]) + page_size - fps, fps);
    buf += fps;

    buf += page_size;
    for (int i = 1; i < num - 1; ++i) {
        memcpy(buf, chman_slice_ptr(arr[i]), page_size);
        buf += page_size;
    }

    size_t lps = (offset + count) % page_size;
    if(!lps) {
        lps = page_size;
    }
    memcpy(buf, chman_slice_ptr(arr[num - 1]), lps);


    return count;
}

/*
 * Returns -1 on failure
 */
ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset) {
    handle* hp = (handle*) mf;
    off_t page_size =hp->sys_page_size;

    unsigned f =  offset / page_size;
    unsigned l = ( offset + count)/ page_size;

    int num = f - l + 1;

    chman_slice** arr = calloc(sizeof(chman_slice*), num);

    chman_up_bunch(hp->manager, f, num, arr);

//--------------
    size_t fps = offset % page_size;
    if(!fps) {
        fps = page_size;
    }
    memcpy(chman_slice_ptr(arr[0]) + page_size - fps, buf, fps);
    buf += fps;

    buf += page_size;
    for (int i = 1; i < num - 1; ++i) {
        memcpy(chman_slice_ptr(arr[i]), buf, page_size);
        buf += page_size;
    }

    size_t lps = (offset + count) % page_size;
    if(!lps) {
        lps = page_size;
    }
    memcpy(chman_slice_ptr(arr[num - 1]), buf, lps);


    return count;
}

/*
 * Returns NULL on failure
 */
void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle) {
    handle* hp = (handle*) mf;
    unsigned f =  offset / hp->sys_page_size;
    unsigned l = ( offset + size)/ hp->sys_page_size;
    chman_slice* s = chman_up_range(hp->manager, f, l - f + 1);

    map_handle* mm = malloc(sizeof(map_handle));
    mm->ptr = chman_slice_ptr(s) + offset % hp->sys_page_size;
    mm->slice_num = l - f + 1;
    mm->slice_first = s;

    return mm->ptr;
}

/*
 * Returns 0 on success and -1 on failure
 */
int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle) {
    handle* hp = mf;
    map_handle* mm  = mapmem_handle;

    chman_slice* s = mm->slice_first;
    for (int i = 0; i < mm->slice_num; ++i) {
        s = chman_down(hp->manager, s);
    }
}

/*
 * Returns -1 on failure
 */
off_t mf_file_size(mf_handle_t mf) {
    handle* hp = mf;
    return hp->info->file_size;
}