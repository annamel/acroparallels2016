#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "mapped_file.h"
#include "hash_table/hash_table.h"

#define HASH_TABLE_SIZE 1000

size_t mem_page_size_g = 0;

typedef struct file_handle_t
{
        int fd;
        size_t size;

        hash_table_t * hash_table;

        void * whole_file_ptr;
} file_handle_t;

/*
 * Returns NULL on failure.
 */
mf_handle_t mf_open(const char * file_path)
{
        if (mem_page_size_g == 0)
                {
                mem_page_size_g = sysconf(_SC_PAGESIZE);
                }

        file_handle_t * file = (file_handle_t *)calloc(1, sizeof(file_handle_t));
        if (file == NULL)
                {
                return MF_OPEN_FAILED;
                }

        // Filling in the file structure
        file->fd = open(file_path, O_RDWR);
        if (file->fd == -1)
                {
                return MF_OPEN_FAILED;
                }


        file->size = lseek(file->fd, 0, SEEK_END);
        void * whole_file_ptr = mmap(NULL, file->size, PROT_READ | PROT_WRITE, MAP_SHARED, file->fd, 0);
        if (whole_file_ptr != MAP_FAILED)
                {
                file->whole_file_ptr = whole_file_ptr;
                return file;
                }

        HASH_TABLE_INIT(file->hash_table);

        file->whole_file_ptr = NULL;
        // size_t size_table = file->size / (mem_page_size_g * MIN_SIZE_CHANK);
        // TODO: Compute better hash table size
        int ret = 0;


        ret = hash_table_construct(&file->hash_table, HASH_TABLE_SIZE);
        if (ret != 0)
                {
                return MF_OPEN_FAILED;
                }

        return file;
}

/*
 * Returns 0 on success and -1 on failure
 */
int mf_close(mf_handle_t mf)
{
        file_handle_t * file = (file_handle_t *)mf;
        if (file == NULL)
                {
                errno = EINVAL;
                return -1;
                }

        // printf("%zu\n", file->size);
        // printf("%p\n", file->whole_file_ptr);
        // printf("%p\n", file->hash_table);


        if (file->whole_file_ptr)
                {
                munmap(file->whole_file_ptr, file->size);
                return 0;
                }

        hash_table_destruct(&file->hash_table);

        close(file->fd);

        free(file);

        return 0;
}

/*
 * Returns -1 on failure
 */
ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset)
{
        file_handle_t * file = (file_handle_t *)mf;
        if (file == NULL)
                {
                errno = EINVAL;
                return -1;
                }

        if (offset > file->size)
                {
                return -1;
                }

        if (offset + count > file->size)
                {
                count = file->size - offset;
                }

        if (file->whole_file_ptr)
                {
                memcpy(buf, file->whole_file_ptr + offset, count);

                return count;
                }

        // printf("Not the whole file is mapped\n");

        // Node *node = find_chank(file, offset, count);
        // if (node == NULL)
        //     return -1;
        // void *ptr = node->value.ptr + offset - node->value.number_first_page*mempagesize;
        // memcpy(buf, ptr, count);
        // }
        // return count;
}

/*
 * Returns -1 on failure
 */
ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset)
{
        file_handle_t * file = (file_handle_t *)mf;
        if (file == NULL)
                {
                errno = EINVAL;
                return -1;
                }

        if (offset > file->size)
                {
                return -1;
                }

        if (offset + count > file->size)
                {
                count = file->size - offset;
                }

        if (file->whole_file_ptr)
                {
                memcpy(file->whole_file_ptr + offset, buf, count);

                return count;
                }

        // printf("Not the whole file is mapped\n");

        // Node *node = find_chank(file, offset, count);
        // if (node == NULL)
        //     return -1;
        // void *ptr = node->value.ptr + offset - node->value.number_first_page*mempagesize;
        // memcpy(buf, ptr, count);
        // }
        // return count;
}

/*
 * Returns NULL on failure
 */
void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle)
{
        file_handle_t * file = (file_handle_t *)mf;
        if (file == NULL)
                {
                errno = EINVAL;
                return NULL;
                }

        if (offset > file->size)
                {
                errno = EINVAL;
                return NULL;
                }

        if (file->whole_file_ptr)
                {
                mapmem_handle = NULL;
                return file->whole_file_ptr + offset;
                }

        // printf("Not the whole file is mapped\n");
        // Node *node = find_chank(file, offset, size);
        // if (node == NULL){
        //     return NULL;
        // }
        // // удаляем из списка чанков с нулевым counter
        // if (++node->value.counter == 1)
        //     ilist_remove(&file->pool.list_zero, node);
        // *mapmem_handle = node;
        // return node->value.ptr + offset - node->value.number_first_page * mempagesize;
}

/*
 * Returns 0 on success and -1 on failure
 */
int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle)
{
        file_handle_t * file = (file_handle_t *)mf;
        if (file == NULL)
                {
                errno = EINVAL;
                return -1;
                }

        if (file->whole_file_ptr)
                {
                return 0;
                }

        // printf("Not the whole file is mapped\n");
        // check_to_NULL(mapmem_handle, -1);
        // Node *node = mapmem_handle;
        // // добавляем в список чанков с нулевым counter
        // if (--node->value.counter == 0){
        //     ilist_append(&file->pool.list_zero, node);
        // }

        // return 0;
}

/*
 * Returns -1 on failure
 */
off_t mf_file_size(mf_handle_t mf)
{
        file_handle_t * file = (file_handle_t *)mf;
        if (file == NULL)
                {
                errno = EINVAL;
                return -1;
                }

        return file->size;
}
