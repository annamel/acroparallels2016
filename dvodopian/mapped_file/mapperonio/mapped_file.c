//
// Created by voddan on 19/05/16.
//

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <memory.h>

#include "mapped_file.h"

typedef struct {
    long            sys_page_size;

    char const *    file_name;
    int             file_descriptor;
    off_t           file_size;


    int             page_num;

//    key_set*        loaded_pages;  // index -> chman_page*

} handle;


typedef struct {
    void * ptr;

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


    return hp;
}

/*
 * Returns 0 on success and -1 on failure
 */
int mf_close(mf_handle_t mf) {
    handle* hp = (handle*) mf;

    close(hp->file_descriptor);


    free(mf);
    return 0;
}

/*
 * Returns -1 on failure
 */
ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset) {
    return 0;
}

/*
 * Returns -1 on failure
 */
ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset) {
    return 0;
}

/*
 * Returns NULL on failure
 */
void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle) {
    return NULL;
}

/*
 * Returns 0 on success and -1 on failure
 */
int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle) {
    return 0;
}

/*
 * Returns -1 on failure
 */
off_t mf_file_size(mf_handle_t mf) {
    return 0;
}