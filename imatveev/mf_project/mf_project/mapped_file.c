//
//  mapped_file.c
//  mf_project
//
//  Created by IVAN MATVEEV on 15.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//


#include "mapped_file.h"
#include "best_error_define_ever.h"

typedef struct file_id {
    int id;
} mf_file_id;

typedef struct mf_mapmem_id {
    mf_file_id file;
    off_t off;
    size_t size;
} mf_mapmem_id;

typedef struct mf_file_struct {
    mf_file_id file;
    size_t max_page_usage;
    HashTable *table;
    HeaderList *pool;
    size_t max_size_pool;
} mf_file_struct;

int mf_my_close(mf_file_id file);

// global const
const unsigned long long default_max_memory_usage = 4294967295LL; // 4 Gb
const size_t default_size_array_of_files = 10;
const size_t default_size_hash_table = 10;
const size_t default_size_pool = 500;
// global variable
size_t mempagesize = 0;
mf_file_struct *array_of_files = NULL;
size_t quantity_of_files = 0;
size_t size_array_of_files = 0;



void deinit_library(void) {
    int i;
    for (i = 0; i < quantity_of_files; i++)
        mf_my_close(array_of_files[i].file);
    free(array_of_files);
}
int check_array_of_files(void){
    if (array_of_files == NULL) {
        
        array_of_files = malloc(default_size_array_of_files*sizeof(mf_file_struct));
        error_with_malloc(array_of_files == NULL, -1);
        atexit(deinit_library); // похоже память освобождается раньше чем вызывается atexit
        size_array_of_files = default_size_array_of_files;
        quantity_of_files = 0;
    } else
    if (quantity_of_files == size_array_of_files) {
        quantity_of_files *= 2;
        array_of_files = realloc(array_of_files, quantity_of_files);
        error_with_malloc(array_of_files == NULL, -1);
    }
    return 0;
}
mf_handle_t mf_open(const char* pathname, size_t max_memory_usage) {
    if (mempagesize == 0)
        mempagesize = sysconf(_SC_PAGESIZE);
    if (check_array_of_files() < 0)
        return MF_OPEN_FAILED;
    
    mf_file_id *file = malloc(sizeof(mf_file_id));
    error_with_malloc(file == NULL, MF_OPEN_FAILED);
    errno = 0;
    file->id = open(pathname, O_RDWR | O_CREAT);
    if (errno != 0)
        return MF_OPEN_FAILED;
    
    mf_file_struct file_struct;
    file_struct.file.id = file->id;
    max_memory_usage = (max_memory_usage == 0) ? default_max_memory_usage : max_memory_usage;
    file_struct.max_page_usage = max_memory_usage/mempagesize;
    off_t size_file = lseek(file_struct.file.id, 0, SEEK_END);
    size_t size_table = (default_size_hash_table > size_file/mempagesize/10) ?
                                default_size_hash_table : size_file/mempagesize/10;
    file_struct.table = initHashTable(size_table);
    if (file_struct.table == NULL){
        free(file);
        errno = ENOMEM;
        return NULL;
    }
    size_t max_size_pool = (file_struct.max_page_usage < default_size_pool) ?
                        file_struct.max_page_usage : default_size_pool;
    file_struct.max_size_pool = max_size_pool;
    file_struct.pool = emptyList();
    if (file_struct.pool == NULL){
        removeHashTable(file_struct.table);
        free(file);
        errno = ENOMEM;
        return MF_OPEN_FAILED;
    }
    array_of_files[quantity_of_files] = file_struct;
    quantity_of_files++;
    
    return file;
}
int mf_my_close(mf_file_id file){
    int i;
    for (i = 0; i < quantity_of_files && array_of_files[i].file.id != file.id; i++);
    if (i == quantity_of_files)
        return -1;
    removeHashTable(array_of_files[i].table);
    removeList(array_of_files[i].pool);
    quantity_of_files--;
    for (; i < quantity_of_files; i++)
        array_of_files[i] = array_of_files[i+1];
    return 0;
}
int mf_close(mf_handle_t mf) {
    if (mf == NULL)
        return -1;
    mf_file_id file = *(mf_file_id *)mf;
    free(mf);
    return mf_my_close(file);
}
ssize_t mf_file_size(mf_handle_t mf) {
    if (mf == NULL)
        return -1;
    mf_file_id *file = (mf_file_id *)mf;
    return lseek(file->id, 0, SEEK_END);
}
mf_file_struct *find_file(mf_handle_t mf){
    if (mf == NULL)
        return NULL;
    mf_file_id *file = (mf_file_id *)mf;
    int i;
    for (i = 0; i < quantity_of_files && array_of_files[i].file.id != file->id; i++);
    if (i == quantity_of_files)
        return NULL;
    return &array_of_files[i];
}
// ищет нужный чанк, если не находит - создает с нулевым counter
// возвращает указатель на чанк который находится в хеш-таблице
Data * find_chunk(mf_file_struct *file_struct, off_t offset, size_t size){
    if (file_struct == NULL)
        return NULL;
    size_t number_first_page = offset/mempagesize;
    size_t size_in_page = size/mempagesize + 1;
    Data *data = getFromHashTable(file_struct->table, number_first_page, size_in_page);
    if (data == NULL){
        Data value;
        value.counter = 0;
        value.number_first_page = number_first_page;
        value.size_in_pages = size_in_page;
        errno = 0;
        value.ptr = mmap(NULL, size_in_page*mempagesize, PROT_READ | PROT_WRITE, MAP_SHARED, file_struct->file.id, number_first_page*mempagesize);
        if (value.ptr == (void *)(-1))
            return NULL;
        if (appendInHashTable(file_struct->table, value) < 0)
            return NULL;
        data = getFromHashTable(file_struct->table, number_first_page, size_in_page);
    }
    return data;
}
ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void* buf) {
    mf_file_struct *file_struct = find_file(mf);
    if (file_struct == NULL)
        return -1;
    Data *data = find_chunk(file_struct, offset, size);
    if (data == NULL)
        return -2;
    if (data->counter == 0)
        if (appendNode(file_struct->pool, *data) < 0)
            return -3;
    if (sizeList(file_struct->pool) >= file_struct->max_size_pool){
        size_t number_first_page;
        size_t size_in_page;
        if (removeFirstNode(file_struct->pool, &number_first_page, &size_in_page) == 0)
            removeFromHashTable(file_struct->table, number_first_page, size_in_page);
    }
    void *pointer = data->ptr + offset % mempagesize;
    if (buf != memcpy(buf, pointer, size))
        return -4;
    return size;
}
ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void* buf) {
    if (mf == NULL)
        return -1;
    mf_file_id *file = (mf_file_id *)mf;
    if (offset != lseek(file->id, offset, SEEK_SET))
        return -1;
    return write(file->id, buf, size);
}
mf_mapmem_t* mf_map(mf_handle_t mf, off_t offset, size_t size) {
    if (mf == NULL)
        return NULL;
    mf_file_struct *file_struct = find_file(mf);
    if (file_struct == NULL)
        return NULL;
    Data *data = find_chunk(file_struct, offset, size);
    if (data == NULL)
        return NULL;
    if (data->counter == 0)
        removeNode(file_struct->pool, data->number_first_page, data->size_in_pages, 0);
    data->counter++;
    mf_mapmem_t* mapmem = malloc(sizeof(mf_mapmem_t));
    error_with_malloc(mapmem == NULL, MF_OPEN_FAILED);
    mapmem->ptr = data->ptr + offset % mempagesize;
    mf_mapmem_id *mapmem_id = malloc(sizeof(mf_mapmem_id));
    if (mapmem_id == NULL){
        free(mapmem);
        errno = ENOMEM;
        return MF_OPEN_FAILED;
    }
    mapmem_id->file.id = file_struct->file.id;
    mapmem_id->off = offset;
    mapmem_id->size = size;
    mapmem->handle = mapmem_id;
    return mapmem;
}
int mf_unmap(mf_mapmem_t* mm){
    if (mm == NULL)
        return -1;
    if (mm->handle == NULL)
        return -2;
    mf_mapmem_id *mapmem_id = mm->handle;
    mf_file_struct *file_struct = find_file(&mapmem_id->file);
    if (file_struct == NULL)
        return -1;
    Data* data = find_chunk(file_struct, mapmem_id->off, mapmem_id->size);
    if (data->counter > 0) {
        if (--data->counter == 0)
            appendNode(file_struct->pool, *data);
    }
    else
        return -3;
    return 0;
}









