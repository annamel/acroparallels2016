//
// Created by voddan on 19/05/16.
//

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <memory.h>
#include <sys/mman.h>

#include "mapped_file.h"

typedef struct {
    long            sys_page_size;

    char const *    file_name;
    int             file_descriptor;
    off_t           file_size;

    void* mem;
} file_handle;


int map(file_handle * hp) ;
int unmap(file_handle * hp) ;


/*
 * Returns NULL on failure.
 */
mf_handle_t mf_open(const char *pathname) {
    int fd = open(pathname, O_RDWR);
    if(-1 == fd) return NULL;

    file_handle* hp = malloc(sizeof(file_handle));
    if(!hp) return NULL;  // close

    hp->sys_page_size = sysconf(_SC_PAGE_SIZE);
    if(hp->sys_page_size <= 0) return NULL;  // close

    hp->file_name = pathname;
    hp->file_descriptor = fd;

    struct stat file_stat;
    fstat(fd, &file_stat);
    hp->file_size = file_stat.st_size;

    hp->mem = NULL;

    return hp;
}

/*
 * Returns 0 on success and -1 on failure
 */
int mf_close(mf_handle_t mf) {
    file_handle* hp = (file_handle*) mf;
    if(!hp) return -1;

    int r1 = unmap(hp);
    int r2 = close(hp->file_descriptor);

    free(hp);
    return (0 == r1 && 0 == r2) ? 0 : -1;
}

/*
 * Returns -1 on failure
 */
ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset) {
    file_handle* hp = (file_handle*) mf;
    if(!hp) return -1;
    if(offset < 0) return -1;  // wtf?

    if(offset + count > hp->file_size) return -1;


    if(map(hp)) return -1;

    memcpy(buf, hp->mem + offset, count);

    return count;
}

/*
 * Returns -1 on failure
 */
ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset) {
    file_handle* hp = (file_handle*) mf;
    if(!hp) return -1;
    if(offset < 0) return -1;  // wtf?

    if(offset + count > hp->file_size) return -1;


    if(map(hp)) return -1;

    memcpy(hp->mem + offset, buf, count);

    return count;
}

/*
 * Returns NULL on failure
 */
void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle) {
    file_handle* hp = (file_handle*) mf;
    if(!hp) return NULL;
    if(offset < 0) NULL;  // wtf?

    if(offset + size > hp->file_size) return NULL;


    if(map(hp)) return NULL;

    *mapmem_handle = NULL;

    return hp->mem + offset;
}

/*
 * Returns 0 on success and -1 on failure
 */
int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle) {
    file_handle* hp = (file_handle*) mf;
    if(!hp) return -1;

    // DO NOTHING

    return 0;
}

/*
 * Returns -1 on failure
 */
off_t mf_file_size(mf_handle_t mf) {
    file_handle* hp = (file_handle*) mf;
    if(!hp) return -1;

    return hp->file_size;
}

/////////////
// PRIVATE //
/////////////

/* it's ok to have hp->mem in the wrong state */

/* if failed returns -1, if success returns 0 */

int map(file_handle * hp) {
    if(hp->mem) return 0;
    hp->mem = mmap(NULL, (size_t) hp->file_size, PROT_WRITE, MAP_SHARED, hp->file_descriptor, 0);

    return (MAP_FAILED != hp->mem) ? 0 : -1;
}

int unmap(file_handle * hp) {
    if(!hp->mem) return 0;

    int r = munmap(hp->mem, (size_t) hp->file_size);
    hp->mem = NULL;

    return r;
}