//
//  mapped_file.c
//  second_try_mf_lib
//
//  Created by IVAN MATVEEV on 23.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#include "mapped_file.h"
#include "hash_table.h"
#include "pool_object.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

const size_t MIN_SIZE_CHANK = 200;
const size_t INIT_SIZE_ARRAY_OF_FILES = 10;
size_t mempagesize = 0;

typedef struct File{
    int id;
    size_t size;
    int flag_mmap_all;      // удалось ли заммапить весь файл (0 если нет)
    void *ptr_all;          // указатель на весь отображенный файл
    PoolObject pool;
} File;

typedef struct ArrayFiles{
    File *files;
    size_t alloced;
    size_t used;
} ArrayFiles;

// mf_handle_t - указатель на структуру File
mf_handle_t mf_open(const char *pathname){
    if (mempagesize == 0)
        mempagesize = sysconf(_SC_PAGESIZE);
    int id = open(pathname, O_RDWR);
    if (id < 0) return MF_OPEN_FAILED;
    File *file = malloc(sizeof(File));
    file->id = id;
    file->size = lseek(id, 0, SEEK_END);
    void * ptr = mmap(NULL, file->size, PROT_READ | PROT_WRITE, MAP_SHARED, id, 0);
    if (ptr != (void *)(-1)){
        file->flag_mmap_all = 1;
        file->ptr_all = ptr;
    } else {
        file->flag_mmap_all = 0;
        size_t size_table = file->size / (mempagesize * MIN_SIZE_CHANK);
        if (init_pool_object(&file->pool, size_table) < 0)
            return MF_OPEN_FAILED;
    }
    return file;
}
int mf_close(mf_handle_t mf){
    File *file = mf;
    if (file == NULL)
        return -1;
    if (file->flag_mmap_all == 0)
        _deinit_pool_object(&file->pool);
    else {
        munmap(file->ptr_all, file->size);
    }
    close(file->id);
    free(file);
    return 0;
}
//  как жаль что нет лямбда функций
size_t number_first_page = 0;
size_t size_in_page = 0;
int check_data(Data data){
    return (data.number_first_page <= number_first_page &&
            data.size_in_pages + data.number_first_page >= size_in_page + number_first_page);
}

Node * find_chank(File *file, off_t offset, size_t size){
    if (size > file->size - offset)
        size = file->size - offset;
    number_first_page = offset / mempagesize;
    size_in_page = (offset + size) / mempagesize - number_first_page + 1;
    Node * node = pool_find(&file->pool, number_first_page, check_data);
    if (node == NULL){
        Data data;
        data.counter = 0;
        data.number_first_page = number_first_page;
        data.size_in_pages = (size_in_page < MIN_SIZE_CHANK) ? MIN_SIZE_CHANK : size_in_page;
        off_t new_offset = number_first_page * mempagesize;
        size_t new_size = data.size_in_pages * mempagesize;
        data.ptr = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, file->id, new_offset);
        // если не удалось, то освобождаем пространство и пробуем еще раз
        if (data.ptr == (void *)(-1)){
            pool_free_space(&file->pool);
            data.ptr = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, file->id, new_offset);
            if (data.ptr == (void *)(-1)){
                errno = ENOMEM;
                return NULL;
            }
        }
        node = pool_append(&file->pool, data);
    }
    return node;
}
#define check_to_NULL(mf, ret_val)\
    do {                          \
        if ((mf) == NULL){        \
            errno = EINVAL;       \
            return (ret_val);     \
        }                         \
    } while(0);
void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle){
    check_to_NULL(mf, NULL);
    File *file = mf;
    if (file->flag_mmap_all){
        if (offset > file->size){
            errno = EINVAL;
            return NULL;
        }
        mapmem_handle = NULL;
        return file->ptr_all + offset;
    } else {
        Node *node = find_chank(file, offset, size);
        if (node == NULL){
            return NULL;
        }
        // удаляем из списка чанков с нулевым counter
        if (++node->value.counter == 1)
            ilist_remove(&file->pool.list_zero, node);
        *mapmem_handle = node;
        return node->value.ptr + offset - node->value.number_first_page * mempagesize;
    }
}
int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle){
    check_to_NULL(mf, -1);
    File *file = mf;
    if (file->flag_mmap_all)
        return 0;
    else {
        check_to_NULL(mapmem_handle, -1);
        Node *node = mapmem_handle;
        // добавляем в список чанков с нулевым counter
        if (--node->value.counter == 0){
            ilist_append(&file->pool.list_zero, node);
        }
    }
    return 0;
}
ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset){
    check_to_NULL(mf, -1);
    File *file = mf;
    if (offset > file->size)
        return -1;
    if (offset + count > file->size)
        count = file->size - offset;
    if (file->flag_mmap_all){
        void *ptr = file->ptr_all + offset;
        memcpy(buf, ptr, count);
    } else {
        Node *node = find_chank(file, offset, count);
        if (node == NULL)
            return -1;
        void *ptr = node->value.ptr + offset - node->value.number_first_page*mempagesize;
        memcpy(buf, ptr, count);
    }
    return count;
}
ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset){
    check_to_NULL(mf, -1);
    File *file = mf;
    if (offset > file->size)
        return -1;
    if (offset + count > file->size)
        count = file->size - offset;
    if (file->flag_mmap_all){
        void *ptr = file->ptr_all + offset;
        memcpy(ptr, buf, count);
    } else {
        Node *node = find_chank(file, offset, count);
        if (node == NULL)
            return -1;
        void *ptr = node->value.ptr + offset - node->value.number_first_page*mempagesize;
        memcpy(ptr, buf, count);
    }
    return count;
}
off_t mf_file_size(mf_handle_t mf){
    check_to_NULL(mf, -1);
    File *file = mf;
    return file->size;
}





