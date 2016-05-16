#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/errno.h>
#include <stdint.h>

#include "ring_buffer.h"

#define POISON_VALUE 0xDEAD

struct ring_buffer
        {
        uint32_t size;

        ring_buffer_elem_t * buffer[RING_BUFFER_SIZE];

        uint32_t head;
        uint32_t tail;

        uint32_t size_of_elem;

        uint32_t count;

        pthread_mutex_t lock;
        };

// Internal functions >>>

ret_code_t _ring_buffer_inc(uint32_t * x, uint32_t size);

// >>>

ret_code_t ring_buffer_construct(ring_buffer_t ** ring_buffer_ptr, uint32_t size_of_elem)
        {
        if (!ring_buffer_ptr || size_of_elem <= 0)
                {
                return WRONG_ARGUMENTS;
                }

        ret_code_t ret_code = ERROR;

        ring_buffer_t * new_ring_buffer = (ring_buffer_t * )calloc(1, sizeof(ring_buffer_t));
        if (!new_ring_buffer)
                {
                fprintf(stderr, "ring_buffer: failed to allocate memory for new instance. Reason: %s\n", strerror(errno));
                goto error_mem_alloc;
                }

        new_ring_buffer->size = sizeof(new_ring_buffer->buffer) / sizeof(new_ring_buffer->buffer[0]);

        memset(new_ring_buffer->buffer, 0, new_ring_buffer->size);

        new_ring_buffer->head = 0;
        new_ring_buffer->tail = 0;

        new_ring_buffer->size_of_elem = size_of_elem;

        new_ring_buffer->count = 0;

        int ret = 0;
        ret = pthread_mutex_init(&(new_ring_buffer->lock), NULL);
        if (ret)
                {
                fprintf(stderr, "ring_buffer: failed to initialize mutex. Reason: %s\n", strerror(ret));
                goto error_mutex_init;
                }

        *ring_buffer_ptr = new_ring_buffer;

        ret_code = SUCCESS;

        // Bottom section >>>

        error_mutex_init:
                if (ret_code == ERROR)
                        {
                        free(new_ring_buffer);
                        new_ring_buffer = NULL;
                        }

        error_mem_alloc:

        return ret_code;
        }



ret_code_t ring_buffer_destruct(ring_buffer_t ** ring_buffer_ptr)
        {
        if (!ring_buffer_ptr)
                {
                return WRONG_ARGUMENTS;
                }

        ring_buffer_t * ring_buffer  = *ring_buffer_ptr;
        ring_buffer_elem_t ** buffer = ring_buffer->buffer;

        int i = 0;
        for (i = 0; i < ring_buffer->size; i++)
                {
                free(buffer[i]);
                buffer[i] = NULL;
                }

        ring_buffer->size = POISON_VALUE;

        ring_buffer->head = POISON_VALUE;
        ring_buffer->tail = POISON_VALUE;


        ring_buffer->count = POISON_VALUE;

        int ret = 0;

        ret = pthread_mutex_destroy(&(ring_buffer->lock));
        if (ret)
                {
                fprintf(stderr, "ring_buffer: failed to destroy mutex. Reason: %s\n", strerror(ret));
                return ERROR;
                }

        free(ring_buffer);
        ring_buffer = NULL;

        return SUCCESS;
        }

ret_code_t ring_buffer_put(ring_buffer_t * ring_buffer, const ring_buffer_elem_t * new_elem_ptr, double * load)
        {
        if (!ring_buffer || !new_elem_ptr)
                {
                return WRONG_ARGUMENTS;
                }

        ret_code_t ret_code = ERROR;

        int ret = 0;
        void * ret_ptr = NULL;

        ring_buffer_elem_t ** buffer = ring_buffer->buffer;
        uint32_t size                = ring_buffer->size;

        // Critical zone
        ret = pthread_mutex_lock(&(ring_buffer->lock));
        if (ret)
                {
                fprintf(stderr, "ring_buffer: failed to lock mutex. Reason: %s\n", strerror(ret));
                goto error_mutex_lock;
                }


        if (buffer[ring_buffer->head] == NULL)
                {
                buffer[ring_buffer->head] = (ring_buffer_elem_t *)calloc(1, ring_buffer->size_of_elem);
                if (!buffer[ring_buffer->head])
                        {
                        fprintf(stderr, "ring_buffer: failed to allocate memory for new element. Reason: %s\n", strerror(errno));
                        goto error_mem_alloc;
                        }
                }

        ret_ptr = memcpy((void *)buffer[ring_buffer->head], (void *)new_elem_ptr, ring_buffer->size_of_elem);
        if (!ret_ptr)
                {
                fprintf(stderr, "ring_buffer: failed to copy new element to the ring buffer\n");
                goto error_mem_copy;
                }

        _ring_buffer_inc(&ring_buffer->head, size);
        if (ring_buffer->head == ring_buffer->tail)
                {
                _ring_buffer_inc(&ring_buffer->tail, size);

                if (load != NULL)
                        {
                        *load = 1; // Ring buffer is full
                        }
                }
        else
                {
                ring_buffer->count++;

                if (load != NULL)
                        {
                        *load = (double)ring_buffer->count / size;
                        }
                }

        ret_code = SUCCESS;

        // Bottom section >>>

        error_mem_copy:

        error_mem_alloc:

        ret = pthread_mutex_unlock(&(ring_buffer->lock));
        if (ret)
                {
                fprintf(stderr, "ring_buffer: failed to unlock mutex. Reason: %s\n", strerror(ret));
                goto error_mutex_unlock;
                }
        //

        error_mutex_lock:

        error_mutex_unlock:

        if (ret_code == ERROR)
                {
                free(buffer[ring_buffer->head]);
                buffer[ring_buffer->head] = NULL;
                }

        return ret_code;

        }

ret_code_t ring_buffer_get(ring_buffer_t * ring_buffer, ring_buffer_elem_t * elem_ptr)
        {
        if (!ring_buffer || !elem_ptr)
                {
                return WRONG_ARGUMENTS;
                }

        ret_code_t ret_code = ERROR;

        int ret = 0;
        void * ret_ptr = NULL;

        ring_buffer_elem_t ** buffer = ring_buffer->buffer;
        uint32_t size                = ring_buffer->size;

        // Critical zone
        ret = pthread_mutex_lock(&(ring_buffer->lock));
        if (ret)
                {
                fprintf(stderr, "ring_buffer: failed to lock mutex. Reason: %s\n", strerror(ret));
                goto error_mutex_lock;
                }

        if ((ring_buffer->tail == ring_buffer->head) || (buffer[ring_buffer->tail] == NULL))
                {
                //fprintf(stderr, "ring_buffer: failed to get element. Reason: ring buffer is empty\n");
                goto error_empty_ring_buffer;
                }

        ret_ptr = memcpy((void *)elem_ptr, (void *)buffer[ring_buffer->tail], ring_buffer->size_of_elem);
        if (!ret_ptr)
                {
                fprintf(stderr, "ring_buffer: failed to copy element to get from the ring buffer\n");
                goto error_mem_copy;
                }

        _ring_buffer_inc(&ring_buffer->tail, size);
        ring_buffer->count--;

        ret_code = SUCCESS;

        // Bottom section >>>

        error_empty_ring_buffer:

        error_mem_copy:

        ret = pthread_mutex_unlock(&(ring_buffer->lock));
        if (ret)
                {
                fprintf(stderr, "ring_buffer: failed to unlock mutex. Reason: %s\n", strerror(ret));
                goto error_mutex_unlock;
                }
        //

        error_mutex_lock:

        error_mutex_unlock:

        return ret_code;
        }

ret_code_t ring_buffer_getn(ring_buffer_t * ring_buffer, ring_buffer_elem_t ** elem_arr_ptr, uint32_t n, uint32_t * read_n)
        {
        if (!ring_buffer || !elem_arr_ptr)
                {
                return WRONG_ARGUMENTS;
                }

        ret_code_t ret_code = ERROR;

        int ret = 0;
        void * ret_ptr = NULL;

        ring_buffer_elem_t ** buffer = ring_buffer->buffer;
        uint32_t size                = ring_buffer->size;
        uint32_t size_of_elem        = ring_buffer->size_of_elem;

        // Critical zone
        ret = pthread_mutex_lock(&(ring_buffer->lock));
        if (ret)
                {
                fprintf(stderr, "ring_buffer: failed to lock mutex. Reason: %s\n", strerror(ret));
                goto error_mutex_lock;
                }

        if ((ring_buffer->tail == ring_buffer->head) || (buffer[ring_buffer->tail] == NULL))
                {
                //fprintf(stderr, "ring_buffer: failed to get element. Reason: ring buffer is empty\n");
                goto error_empty_ring_buffer;
                }

        uint32_t tail_iter = ring_buffer->tail;
        uint32_t head      = ring_buffer->head;

        int i = 0;
        while ((tail_iter != head) && (i != n))
                {
                ret_ptr = memcpy((void *)elem_arr_ptr[i], (void *)buffer[tail_iter], size_of_elem);
                if (!ret_ptr)
                        {
                        fprintf(stderr, "ring_buffer: failed to copy element to get from the ring buffer\n");
                        goto error_mem_copy;
                        }

                _ring_buffer_inc(&tail_iter, size);
                i++;
                }

        ring_buffer->tail   = tail_iter;
        ring_buffer->count -= i;

        if (read_n != NULL)
                {
                *read_n = (uint32_t)i;
                }

        ret_code = SUCCESS;

        // Bottom section >>>

        error_empty_ring_buffer:

        error_mem_copy:

        ret = pthread_mutex_unlock(&(ring_buffer->lock));
        if (ret)
                {
                fprintf(stderr, "ring_buffer: failed to unlock mutex. Reason: %s\n", strerror(ret));
                goto error_mutex_unlock;
                }
        //

        error_mutex_lock:

        error_mutex_unlock:

        return ret_code;
        }

ret_code_t ring_buffer_print(ring_buffer_t * ring_buffer, void (* printer)(const ring_buffer_elem_t *))
        {
        if (!ring_buffer || !printer)
                {
                return WRONG_ARGUMENTS;
                }

        int ret = 0;

        ring_buffer_elem_t ** buffer = ring_buffer->buffer;
        uint32_t size = ring_buffer->size;

        // Critical zone
        ret = pthread_mutex_lock(&(ring_buffer->lock));
        if (ret)
                {
                fprintf(stderr, "ring_buffer: failed to lock mutex. Reason: %s\n", strerror(ret));
                return ERROR;
                }

        uint32_t tail_iter = ring_buffer->tail;
        uint32_t head      = ring_buffer->head;

        while (tail_iter != head)
                {
                if (buffer[tail_iter] != NULL)
                        {
                        printer(buffer[tail_iter]);
                        }

                _ring_buffer_inc(&tail_iter, size);
                }

        ret = pthread_mutex_unlock(&(ring_buffer->lock));
        if (ret)
                {
                fprintf(stderr, "ring_buffer: failed to unlock mutex. Reason: %s\n", strerror(ret));
                return ERROR;
                }
        //

        return SUCCESS;
        }

ret_code_t _ring_buffer_inc(uint32_t * x, uint32_t size)
        {
        *x = (*x + 1) % size;
        return SUCCESS;
        }
