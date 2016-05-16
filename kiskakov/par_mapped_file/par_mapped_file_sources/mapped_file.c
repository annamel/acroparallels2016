#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "mapped_file.h"
#include "hash_table/hash_table.h"

#define HASH_TABLE_SIZE 5
#define CHUNK_N 1024

size_t mem_page_size_g = 0;

typedef struct file_handle_t
{
        int fd;
        size_t size;

        hash_table_t * hash_table;
        chunk_handle_t * chunks;
        pthread_mutex_t lock;

        void * whole_file_ptr;
} file_handle_t;

int search_chunk_idx(file_handle_t * file, off_t offset, size_t count, int calced);
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

        pthread_mutex_init(&file->lock, NULL);

        file->chunks = malloc(CHUNK_N * sizeof(chunk_handle_t));
        if (!file->chunks)
                {
                return MF_OPEN_FAILED;
                }
        int i = 0;
        for (i = 0; i < CHUNK_N; i++)
                {
                file->chunks[i].ptr = NULL;

                file->chunks[i].page_offset = -1;
                file->chunks[i].page_size = -1;

                file->chunks[i].ref_counter = 0;
                }

        file->size = lseek(file->fd, 0, SEEK_END);
        void * whole_file_ptr = mmap(NULL, file->size, PROT_READ | PROT_WRITE, MAP_SHARED, file->fd, 0);
        // whole_file_ptr = MAP_FAILED; // DELETE IT
        if (whole_file_ptr != MAP_FAILED)
                {
                file->whole_file_ptr = whole_file_ptr;
                return file;
                }

        //HASH_TABLE_INIT(file->hash_table);

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

void print(mf_handle_t mf, int n)
{

        file_handle_t * file = (file_handle_t *)mf;
        int i = 0;
        for (i = 0; i < n; i++)
                {
                chunk_handle_t * chunks = file->chunks;
                printf("===== %d =====\n", i);
                printf("ptr = %p\n", chunks[i].ptr);
                printf("page_offset = %d\n", (int)chunks[i].page_offset);
                printf("page_size = %d\n", (int)chunks[i].page_size);
                printf("ref = %d\n", (int)chunks[i].ref_counter);

                printf("=====\n");
                }
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

        pthread_mutex_lock(&file->lock);

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

        free(file->chunks);

        pthread_mutex_unlock(&file->lock);

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

        pthread_mutex_lock(&file->lock);

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

        int chunk_idx = 0;
        chunk_idx = search_chunk_idx(file, offset, count, 0);
        if (chunk_idx == -1)
                {
                errno = ENOMEM;
                return -1;
                }

        memcpy(buf, file->chunks[chunk_idx].ptr + offset - file->chunks[chunk_idx].page_offset * mem_page_size_g, count);

        pthread_mutex_unlock(&file->lock);

        return count;
}

int search_chunk_idx(file_handle_t * file, off_t offset, size_t count, int calced)
{

        int page_offset = offset;
        int page_size = count;

        if (calced == 0)
        {
                page_offset = offset / mem_page_size_g;
                page_size = (offset + count) / mem_page_size_g - page_offset + 1;
        }


        chunk_handle_t chunk;
        chunk.ptr = NULL;

        int i = 0;
        for (i = 0; i < CHUNK_N; i++)
                {
                chunk_handle_t current_chunk = file->chunks[i];
                if (page_offset == current_chunk.page_offset)
                        {
                        if (current_chunk.page_offset <= page_offset && current_chunk.page_size + current_chunk.page_offset >= page_size + page_offset)
                                {
                                // memcpy(&chunk, &current_chunk, sizeof(chunk_handle_t));
                                return i;
                                }
                        }
                }


        chunk.ref_counter = 0;

        chunk.page_offset = page_offset;
        chunk.page_size = page_size;

        off_t new_offset = page_offset * mem_page_size_g;
        size_t new_size = page_size * mem_page_size_g;

        chunk.ptr = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, file->fd, new_offset);

        int free_idx = -1;
        for (i = 0; i < CHUNK_N; i++)
        {
        if (file->chunks[i].ptr == NULL)
                {
                free_idx = i;
                break;
                }
        }

        if (free_idx != -1)
                {
                memcpy(&file->chunks[free_idx], &chunk, sizeof(chunk_handle_t));
                return free_idx;
                }
        else
                {
                for (i = 0; i < CHUNK_N; i++)
                        {
                        if (file->chunks[i].ref_counter == 0)
                                {
                                munmap(file->chunks[i].ptr, file->chunks[i].page_size);
                                memcpy(&file->chunks[i], &chunk, sizeof(chunk_handle_t));
                                return i;
                                break;
                                }
                        }
                }

        return -1;
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

        pthread_mutex_lock(&file->lock);

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

        int chunk_idx = search_chunk_idx(file, offset, count, 0);
        if (chunk_idx == -1)
                {
                errno = ENOMEM;
                return -1;
                }

        memcpy(file->chunks[chunk_idx].ptr + offset - file->chunks[chunk_idx].page_offset * mem_page_size_g, buf, count);

        pthread_mutex_unlock(&file->lock);

        return count;
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

        pthread_mutex_lock(&file->lock);

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

        int chunk_idx = search_chunk_idx(file, offset, size, 0);
        if (chunk_idx == -1)
                {
                errno = ENOMEM;
                return NULL;
                }

        file->chunks[chunk_idx].ref_counter++;

        // chunk_handle_t * ret_chunk = (chunk_handle_t *)mapmem_handle;
        memcpy((chunk_handle_t *)mapmem_handle, &file->chunks[chunk_idx], sizeof(chunk_handle_t));

        pthread_mutex_unlock(&file->lock);

        return file->chunks[chunk_idx].ptr + offset - file->chunks[chunk_idx].page_offset * mem_page_size_g;
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

        pthread_mutex_lock(&file->lock);

        if (file->whole_file_ptr)
                {
                return 0;
                }
        chunk_handle_t * chunk = (chunk_handle_t *)&mapmem_handle;
        int chunk_idx = search_chunk_idx(file, chunk->page_offset, chunk->page_size, 1);
        if (chunk_idx == -1)
                {
                return -1;
                }

        file->chunks[chunk_idx].ref_counter--;

        pthread_mutex_unlock(&file->lock);

        return 0;
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
