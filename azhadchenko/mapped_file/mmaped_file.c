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
#include "inverted_index.h"

struct Mmaped_file{
    size_t status;
    struct Pool* pool;
    struct Inverted_index* ii;
    size_t chunk_size;
    size_t chunk_count;
    int fd;
    off_t size;
    char bigmmap;
};

#define DEFAULT_CHUNK_SIZE 1024*1024*16

inline void* __mmap_file(int fd, off_t length) {

    void* tmp = mmap(NULL, length, PROT_READ | PROT_WRITE,  MAP_SHARED, fd, 0);
    if(tmp == (void*)-1)
        return 0;

    return tmp;
}

void* mf_open(const char* pathname) {
    if(!pathname)
        return 0;

    int fd = open(pathname, O_RDWR);
    if(fd == -1)
        return 0;

    struct Mmaped_file* mf = (struct Mmaped_file*)calloc(1, sizeof(struct Mmaped_file));
    if(!mf) {
        close(fd);
        return 0;
    }

    mf -> fd = fd;


    struct stat finfo = {0};
    int err = fstat(fd, &finfo);
    if(err == -1)
        return 0;

    size_t chunk_size = DEFAULT_CHUNK_SIZE;
    size_t chunk_count = finfo.st_size / chunk_size;
    while(chunk_count > 1000) {
        chunk_size *= 2;
        chunk_count = finfo.st_size / chunk_size;
    }

    mf -> pool = __mmap_file(fd, finfo.st_size);
    if(mf -> pool) {
        mf -> bigmmap++;
        return 0;
    }


    mf -> pool = init_pool();
    if(!mf -> pool) {
        close(fd);
        free(mf);
        return 0;
    }



    mf -> ii = init_ii(chunk_count);
    if(!mf -> ii) {
        close(fd);
        destruct_pool(mf -> pool);
        free(mf);
        return 0;
    }

    mf -> chunk_size = chunk_size;
    mf -> chunk_count = chunk_count;
    mf -> size = finfo.st_size;

    return mf;
}

int mf_close(void* tmp) {
    struct Mmaped_file* mf = tmp;

    if(!tmp)
        return 0;

    close(mf -> fd);

    if(mf -> bigmmap) {
        munmap(mf -> pool, mf -> size);
        free(mf);
        return 0;
    }

    destruct_ii(mf -> ii);
    destruct_pool(mf -> pool);

    free(mf);

    return 0;
}

void try_free_chunks(struct Mmaped_file* mf) {return;}

#define CALC_SIZE_IN_CHUNKS(length, chunk_size) ( (length % chunk_size == 0) ? length : (length /  chunk_size + 1) * chunk_size )


void* map_chunk(struct Mmaped_file* mf, off_t position, size_t length) {

    void* tmp  = 0;
    position = position / (off_t) mf -> chunk_size * mf -> chunk_size;
    length = CALC_SIZE_IN_CHUNKS(length, mf -> chunk_size);

#ifndef MAP_HUGETLB
#define MAP_HUGETLB 0
#endif // MAP_HUGETLB

    for(int i = 0; i < 5; i++) {


//        tmp = mmap(NULL, length, PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_HUGETLB | MAP_ANONYMOUS, mf -> fd, position);
//        mmap(tmp, length, PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_FIXED, mf -> fd, position);

        tmp = mmap(NULL, length, PROT_READ | PROT_WRITE,  MAP_SHARED, mf -> fd, position);
        if(tmp == (void*)-1)
            try_free_chunks(mf);
        else
            break;

        if(i == 4)
            return 0;
    }

    position = position / mf -> chunk_size;
    length = length / mf -> chunk_size;

    struct Chunk* item = allocate_chunk(mf -> pool, tmp, position, length);
    if(!item)
        return 0;

    int add_res = add_item(mf -> ii, item, position, position + length);
    if(add_res != length)
        return 0;

    return item -> ptr;
}


void* get_ptr(struct Mmaped_file* mf, off_t position, size_t length, void** chunk_addr) { //actually somewhere here must be controlled refcount
    if(!mf)
        return 0;

    struct Ii_element* item = mf -> ii -> data[position / mf -> chunk_size];

    if(!item)
        return map_chunk(mf, position, length);


    struct Chunk* curr_chunk = (struct Chunk*)item->item;

    while(curr_chunk -> offset + curr_chunk -> size > position / mf -> chunk_size + CALC_SIZE_IN_CHUNKS(length, mf -> chunk_size)) {
        if(!item->next)
            return map_chunk(mf, position, length);

        item = item -> next;
        curr_chunk = (struct Chunk*)item->item;
    };

    if(chunk_addr) {
        curr_chunk ->refcount++;
        *chunk_addr = curr_chunk;
    }

    return curr_chunk -> ptr;
}

ssize_t mf_read(void* tmp, void* buf, size_t count, off_t offset) {
    struct Mmaped_file* mf = tmp;
    size_t tmp1 = count;

    if(mf -> bigmmap) {
        void* source = ((void*)mf->pool) + offset;
        memcpy(buf, source, count);
        return count;
    }

    do {

        size_t read_count = mf -> chunk_size - offset % mf -> chunk_size;
        read_count = (count > read_count) ? read_count : count;

        void* source = get_ptr(mf, offset, read_count, 0);
        if(!source)
            return -1;

        source += offset % mf -> chunk_size;
        offset += (offset % mf -> chunk_size == 0)? mf -> chunk_size : offset % mf -> chunk_size;
        count -= read_count;

        memcpy(buf, source, read_count);
    } while(count != 0);

    return tmp1;
}



ssize_t mf_write(void* tmp, void* buf, size_t count, off_t offset) {
    struct Mmaped_file* mf = tmp;
    size_t tmp1 = count;

    if(mf -> bigmmap) {
        void* source = ((void*)mf->pool) + offset;
        memcpy(source, buf, count);
        return count;
    }

    do {

        size_t write_count = mf -> chunk_size - offset % mf -> chunk_size;
        write_count = (count > write_count) ? write_count : count;

        void* source = get_ptr(mf, offset, count, 0);
        if(!source)
            return -1;

        source += offset % mf -> chunk_size;
        offset += (offset % mf -> chunk_size == 0)? mf -> chunk_size : offset % mf -> chunk_size;
        count -= write_count;

        memcpy(source, buf, write_count);
    } while(count != 0);

    return tmp1;
}

void *mf_map(void* tmp, off_t offset, size_t size, void** mapmem_handle) {

    struct Mmaped_file* mf = tmp;
    if(mf -> bigmmap) {
        return mf -> pool + offset;
    }

    return get_ptr(tmp, offset, size, mapmem_handle);
}

int mf_unmap(void* mf, void** mapmem_handle) {
    return 0;
}


off_t mf_file_size(void* mf) {
    return (((struct Mmaped_file*)mf)->size);
}
