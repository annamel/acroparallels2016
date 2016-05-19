#include "ring_buf.h"
#include <malloc.h>
#include <string.h>
#include <pthread.h>

#include <assert.h>


#define _INC_IDX_BY_MODULO(idx, size_of_arr)  ((idx + 1) % size_of_arr)


typedef struct Ring_buf
        {
        size_t size_of_buf;
        /*
        ring_buf_entry_ptr* buf;

        size_t size_of_entry;
        */
        ring_buf_elem_t* buf_of_elems;

        // this var is used to speed up 'get' func in critical section
        size_t max_size_of_entry;

        size_t head;
        pthread_mutex_t head_mtx;
        int is_head_mtx_destroyed;

        size_t tail;
        pthread_mutex_t tail_mtx;
        int is_tail_mtx_destroyed;

        // I will try w/o count_mtx
        size_t count;
        } ring_buf_t;



int Ring_buf_construct (ring_buf_t** ring_buf, size_t ring_buf_size)
{
size_t i = 0;
ring_buf_t* ring_buf_ptr = NULL;

if ((ring_buf && ring_buf_size) == 0)
        {
        my_errno = wrong_args;
        return -1;
        }

if ((ring_buf_ptr = (ring_buf_t*) calloc (1, sizeof (ring_buf_t))) == NULL)
        {
        my_errno = mem_err;
        return -1;
        }

ring_buf_ptr->size_of_buf = ring_buf_size;

if ((ring_buf_ptr->buf_of_elems = (ring_buf_elem_t*) calloc (ring_buf_size, sizeof (ring_buf_elem_t))) == NULL)
        {
        my_errno = mem_err;
        goto error_calloc_of_buf;
        }

// if (all-bits-zero) != NULL
for (i = 0; i < ring_buf_size; i++)
        ring_buf_ptr->buf_of_elems[i].buf_for_entry = NULL;

ring_buf_ptr->is_head_mtx_destroyed = 1;
ring_buf_ptr->is_tail_mtx_destroyed = 1;

ring_buf_ptr->head = 0;
if (pthread_mutex_init (&(ring_buf_ptr->head_mtx), NULL))
        {
        my_errno = mutex_init_err;
        goto error_head_mtx_init;
        }
ring_buf_ptr->is_head_mtx_destroyed = 0;

ring_buf_ptr->tail = 0;
if (pthread_mutex_init (&(ring_buf_ptr->tail_mtx), NULL))
        {
        my_errno = mutex_init_err;
        goto error_tail_mtx_init;
        }
ring_buf_ptr->is_tail_mtx_destroyed = 0;

ring_buf_ptr->count = 0;
ring_buf_ptr->max_size_of_entry = 0;

*ring_buf = ring_buf_ptr;

return 0;


error_tail_mtx_init:
        if (pthread_mutex_destroy (&(ring_buf_ptr->head_mtx)))
                {
                // TODO: try to return this branch in order to call 'destruct'
                my_errno = mutex_destr_err;

                free (ring_buf_ptr->buf_of_elems);
                ring_buf_ptr->buf_of_elems = NULL;

                *ring_buf = ring_buf_ptr;
                return -1; // we need to call 'Ring_buf_destruct'
                }
error_head_mtx_init:
        free (ring_buf_ptr->buf_of_elems);
error_calloc_of_buf:
        free (ring_buf_ptr);
        return -1;
}


int Ring_buf_destruct (ring_buf_t** ring_buf)
{
size_t i = 0;
ring_buf_t* ring_buf_ptr = NULL;

// http://stackoverflow.com/questions/9894013/is-null-always-zero-in-c
// NULL "is guaranteed to compare equal to 0. But it doesn't have to
//      be represented with all-zero bits."
if (ring_buf == NULL)
        {
        my_errno = wrong_args;
        return -1;
        }
else if (*ring_buf == NULL)
        {
        my_errno = wrong_args;
        return -1;
        }

ring_buf_ptr = *ring_buf;

if (ring_buf_ptr->buf_of_elems != NULL)
        {
        for (i = 0; i < (ring_buf_ptr->size_of_buf); i++)
                {
                if (ring_buf_ptr->buf_of_elems[i].buf_for_entry != NULL)
                        {
                        free (ring_buf_ptr->buf_of_elems[i].buf_for_entry);
                        ring_buf_ptr->buf_of_elems[i].buf_for_entry = NULL;
                        }
                }

        free (ring_buf_ptr->buf_of_elems);
        ring_buf_ptr->buf_of_elems = NULL;
        }

if (ring_buf_ptr->is_head_mtx_destroyed == 0)
        {
        if (pthread_mutex_destroy (&(ring_buf_ptr->head_mtx)))
                {
                my_errno = mutex_destr_err;
                return -1;
                }
        ring_buf_ptr->is_head_mtx_destroyed = 1;
        }

if (ring_buf_ptr->is_tail_mtx_destroyed == 0)
        {
        if (pthread_mutex_destroy (&(ring_buf_ptr->tail_mtx)))
                {
                my_errno = mutex_destr_err;
                return -1;
                }
        ring_buf_ptr->is_tail_mtx_destroyed = 1;
        }

free (ring_buf_ptr);
*ring_buf = NULL;

return 0;
}


int Ring_buf_put (ring_buf_ptr ring_buf, const ring_buf_entry_t* ring_buf_entry_ptr, size_t size_of_ring_buf_entry)
{
int ret_val = 0;
void* ret_ptr = 0;
size_t cur_count = 0, head_tmp = 0, tail_tmp = 0, count_tmp = 0;
ring_buf_entry_t *new_buf = NULL, *old_buf = NULL;
ring_buf_elem_t* cur_elem = NULL;

if ((ring_buf && ring_buf_entry_ptr && size_of_ring_buf_entry) == 0)
        {
        my_errno = wrong_args;
        return -1;
        }

if ((new_buf = (ring_buf_entry_t*) calloc (1, size_of_ring_buf_entry)) == NULL)
        {
        my_errno = mem_err;
        return -1;
        }

if ((ret_ptr = memcpy (new_buf, ring_buf_entry_ptr, size_of_ring_buf_entry)) == NULL)
        {
        my_errno = mem_err;
        goto cleanup;
        }

//==============================================================================
// Lock head_mtx
//------------------------------------------------------------------------------
if ((ret_val = pthread_mutex_lock (&(ring_buf->head_mtx))) != 0)
        {
        my_errno = mutex_err;
        goto cleanup;
        }
//------------------------------------------------------------------------------

cur_elem = &(ring_buf->buf_of_elems[ring_buf->head]);

old_buf = cur_elem->buf_for_entry;
cur_elem->buf_for_entry = new_buf;

cur_elem->size_of_entry = size_of_ring_buf_entry;

tail_tmp = __atomic_load_n (&(ring_buf->tail), __ATOMIC_RELAXED);
if (_INC_IDX_BY_MODULO(ring_buf->head, ring_buf->size_of_buf) == tail_tmp)
        {
        // head right behind tail, and we should atomically move tail forward
        //      i.e. "erase" next elem

        //======================================================================
        // Lock tail_mtx
        //----------------------------------------------------------------------
        if ((ret_val = pthread_mutex_lock (&(ring_buf->tail_mtx))) != 0)
                {
                my_errno = mutex_err;
                goto cleanup_w_head_unlock;
                }
        //----------------------------------------------------------------------

        tail_tmp = ring_buf->tail;
        if (_INC_IDX_BY_MODULO(ring_buf->head, ring_buf->size_of_buf) == tail_tmp)
                {
                __atomic_store_n (&(ring_buf->tail),
                        _INC_IDX_BY_MODULO(tail_tmp, ring_buf->size_of_buf), __ATOMIC_RELAXED);
                // also if this happened, we need to atomically decrease the count
                count_tmp = __atomic_sub_fetch(&(ring_buf->count), 1, __ATOMIC_RELAXED);
                }
        // else
        //      it means that at least one 'get' was performed and there is enough
        //      space for this 'put' so we can continue w/o advancing the tail

        //------------------------------------------------------------------------------
        ret_val = pthread_mutex_unlock (&(ring_buf->tail_mtx));
        assert (ret_val == 0); // if != 0, then it is really fatal situation
        //------------------------------------------------------------------------------
        // Unlock tail_mtx
        //==============================================================================
        }

head_tmp = ring_buf->head;
__atomic_store_n (&(ring_buf->head), _INC_IDX_BY_MODULO(head_tmp, ring_buf->size_of_buf), __ATOMIC_RELAXED);

count_tmp = __atomic_add_fetch(&(ring_buf->count), 1, __ATOMIC_RELAXED);
// '- 1' is because 'head' points to the next "free" place in buffer
// count_tmp = ring_buf->count;
// if (count_tmp < (ring_buf->size_of_buf - 1))
//        __atomic_store_n (&(ring_buf->count), count_tmp + 1, __ATOMIC_RELAXED);

if (ring_buf->max_size_of_entry < size_of_ring_buf_entry)
        __atomic_store_n (&(ring_buf->max_size_of_entry), size_of_ring_buf_entry, __ATOMIC_RELAXED);

cur_count = count_tmp;

//------------------------------------------------------------------------------
ret_val = pthread_mutex_unlock (&(ring_buf->head_mtx));
assert (ret_val == 0); // if != 0, then it is really fatal situation
//------------------------------------------------------------------------------
// Unlock head_mtx
//==============================================================================

free (old_buf);

return (cur_count * 100)/(ring_buf->size_of_buf);


cleanup_w_head_unlock:
        ret_val = pthread_mutex_unlock (&(ring_buf->head_mtx));
        assert (ret_val == 0); // if != 0, then it is really fatal situation
cleanup:
        free (new_buf);
        return -1;
}


int Ring_buf_get (const ring_buf_ptr ring_buf, ring_buf_elem_ptr* ring_buf_elem_buf_ptr)
{
int ret_val = 0;
size_t max_size_tmp = 0, head_tmp = 0, tail_tmp = 0;
void* ret_ptr = NULL;
ring_buf_entry_t *new_buf_for_entry = NULL;
ring_buf_elem_t  *new_buf_for_elem = NULL, *cur_elem = NULL;

if ((ring_buf && ring_buf_elem_buf_ptr) == 0)
        {
        my_errno = wrong_args;
        return -1;
        }

head_tmp = __atomic_load_n (&(ring_buf->head), __ATOMIC_RELAXED);
tail_tmp = __atomic_load_n (&(ring_buf->tail), __ATOMIC_RELAXED);

if (head_tmp == tail_tmp)
        {
        my_errno = ring_buf_empty;
        return -1;
        }

if ((new_buf_for_elem = (ring_buf_elem_t*) calloc (1, sizeof (ring_buf_elem_t))) == NULL)
        {
        my_errno = mem_err;
        return -1;
        }

max_size_tmp = __atomic_load_n (&(ring_buf->max_size_of_entry), __ATOMIC_RELAXED);

// if ((new_buf_for_entry = (ring_buf_entry_t*) calloc (1, ring_buf->buf_of_elems[ring_buf->tail].size_of_entry)) == NULL)
if ((new_buf_for_entry = (ring_buf_entry_t*) calloc (1, max_size_tmp)) == NULL)
        {
        my_errno = mem_err;
        goto mem_err_entry_calloc;
        }

//==============================================================================
// Lock tail_mtx
//------------------------------------------------------------------------------
if ((ret_val = pthread_mutex_lock (&(ring_buf->tail_mtx))) != 0)
        {
        my_errno = mutex_err;
        goto cleanup;
        }
//------------------------------------------------------------------------------

cur_elem = &(ring_buf->buf_of_elems[ring_buf->tail]);

if (cur_elem->size_of_entry > max_size_tmp)
        {
        free (new_buf_for_entry);
        new_buf_for_entry = NULL;

        if ((new_buf_for_entry = (ring_buf_entry_t*) calloc (1, max_size_tmp)) == NULL)
                {
                my_errno = mem_err;
                new_buf_for_entry = NULL;
                goto cleanup_w_tail_unlock;
                }
        }

new_buf_for_elem->size_of_entry = cur_elem->size_of_entry;

if ((ret_ptr = memcpy (new_buf_for_entry, cur_elem->buf_for_entry, cur_elem->size_of_entry)) == NULL)
        {
        my_errno = mem_err;
        goto cleanup_w_tail_unlock;
        }
new_buf_for_elem->buf_for_entry = new_buf_for_entry;

tail_tmp = ring_buf->tail;
__atomic_store_n (&(ring_buf->tail), _INC_IDX_BY_MODULO(tail_tmp, ring_buf->size_of_buf), __ATOMIC_RELAXED);

__atomic_sub_fetch(&(ring_buf->count), 1, __ATOMIC_RELAXED);

//------------------------------------------------------------------------------
ret_val = pthread_mutex_unlock (&(ring_buf->tail_mtx));
assert (ret_val == 0); // if != 0, then it is really fatal situation
//------------------------------------------------------------------------------
// Unlock tail_mtx
//==============================================================================

*ring_buf_elem_buf_ptr = new_buf_for_elem;

return 0;


cleanup_w_tail_unlock:
        ret_val = pthread_mutex_unlock (&(ring_buf->tail_mtx));
        assert (ret_val == 0); // if != 0, then it is really fatal situation
cleanup:
        free (new_buf_for_entry);
mem_err_entry_calloc:
        free (new_buf_for_elem);
        return -1;
}


int Ring_buf_delete_elem_after_get (ring_buf_elem_ptr* ring_buf_elem_buffer)
{
if ((ring_buf_elem_buffer && (*ring_buf_elem_buffer)) == 0)
        {
        my_errno = wrong_args;
        return -1;
        }

free ((*ring_buf_elem_buffer)->buf_for_entry);
(*ring_buf_elem_buffer)->buf_for_entry = NULL;

free (*ring_buf_elem_buffer);
(*ring_buf_elem_buffer) = NULL;

return 0;
}

int Ring_buf_get_N_elems (ring_buf_ptr ring_buf, ring_buf_N_elems_ptr* ring_buf_N_elems_buf_ptr, size_t N)
{
size_t i = 0;
int ret_val = 0;
size_t head_tmp = 0, tail_tmp = 0, count_tmp = 0;
void* ret_ptr = NULL;
size_t total_size_of_entries = 0;
ring_buf_entry_t   *new_buf_for_buf_of_entries = NULL;
ring_buf_elem_t    *new_buf_for_buf_of_elems = NULL;
ring_buf_N_elems_t *new_buf_for_N_elems = NULL;

if ((ring_buf && ring_buf_N_elems_buf_ptr && N) == 0)
        {
        my_errno = wrong_args;
        return -1;
        }

head_tmp = __atomic_load_n (&(ring_buf->head), __ATOMIC_RELAXED);
tail_tmp = __atomic_load_n (&(ring_buf->tail), __ATOMIC_RELAXED);

if (head_tmp == tail_tmp)
        {
        my_errno = ring_buf_empty;
        return -1;
        }

if ((new_buf_for_N_elems = (ring_buf_N_elems_t*) calloc (1, sizeof (ring_buf_N_elems_t))) == NULL)
        {
        my_errno = mem_err;
        return -1;
        }

//==============================================================================
// To avoid deadlocks we need to lock mutexes in the same order as in the 'put':
//      first - head_mtx, then - tail_mtx
//==============================================================================
// Lock head_mtx
//------------------------------------------------------------------------------
if ((ret_val = pthread_mutex_lock (&(ring_buf->head_mtx))) != 0)
        {
        my_errno = mutex_err;
        goto cleanup_from_head_mtx;
        }
//------------------------------------------------------------------------------

//==============================================================================
// Lock tail_mtx
//------------------------------------------------------------------------------
if ((ret_val = pthread_mutex_lock (&(ring_buf->tail_mtx))) != 0)
        {
        my_errno = mutex_err;
        goto cleanup_from_tail_mtx;
        }
//------------------------------------------------------------------------------

if (N > ring_buf->count)
        N = ring_buf->count;

new_buf_for_N_elems->numb_of_elems = N;

//----------
// _buf_for_elems
if ((new_buf_for_buf_of_elems = (ring_buf_elem_t*) calloc (N, sizeof (ring_buf_elem_t))) == NULL)
        {
        my_errno = mem_err;
        goto cleanup_from_buf_of_elems_calloc;
        }
new_buf_for_N_elems->buf_of_elems = new_buf_for_buf_of_elems;

for (i = 0; i < N; i++)
        {
        new_buf_for_buf_of_elems[i].size_of_entry =
                ring_buf->buf_of_elems[(ring_buf->tail + i) % (ring_buf->size_of_buf)].size_of_entry;
        total_size_of_entries += new_buf_for_buf_of_elems[i].size_of_entry;
        }
//----------
// _buf_for_entries
//
// allocate contiguous buffer of 'total_size_of_entries' bytes
if ((new_buf_for_buf_of_entries = (ring_buf_entry_t*) calloc (1, total_size_of_entries)) == NULL)
        {
        my_errno = mem_err;
        goto cleanup_from_buf_of_entries_calloc;
        }

new_buf_for_buf_of_elems[0].buf_for_entry = new_buf_for_buf_of_entries;
if ((ret_ptr = memcpy (new_buf_for_buf_of_entries, ring_buf->buf_of_elems[ring_buf->tail].buf_for_entry,
                                                   ring_buf->buf_of_elems[ring_buf->tail].size_of_entry)) == NULL)
        {
        my_errno = mem_err;
        goto cleanup_from_memcpy;
        }
for (i = 1; i < N; i++)
        {
        new_buf_for_buf_of_elems[i].buf_for_entry =
                        new_buf_for_buf_of_elems[(i - 1) % (ring_buf->size_of_buf)].buf_for_entry +
                        new_buf_for_buf_of_elems[(i - 1) % (ring_buf->size_of_buf)].size_of_entry;

        if ((ret_ptr = memcpy (new_buf_for_buf_of_elems[i].buf_for_entry,
                               ring_buf->buf_of_elems[(ring_buf->tail + i) % (ring_buf->size_of_buf)].buf_for_entry,
                               ring_buf->buf_of_elems[(ring_buf->tail + i) % (ring_buf->size_of_buf)].size_of_entry)) == NULL)
                {
                my_errno = mem_err;
                goto cleanup_from_memcpy;
                }
        }
//----------
//      if (ring_buf->tail > ring_buf->head)
//              {
//              tail_tmp = (ring_buf->tail + N) % (ring_buf->size_of_buf);
//              __atomic_store_n (&(ring_buf->tail), tail_tmp, __ATOMIC_RELAXED);
//              }
//      else
//              __atomic_add_fetch (&(ring_buf->tail), N, __ATOMIC_RELAXED);
tail_tmp = (ring_buf->tail + N) % (ring_buf->size_of_buf);
__atomic_store_n (&(ring_buf->tail), tail_tmp, __ATOMIC_RELAXED);

count_tmp = ring_buf->count - N;
__atomic_store_n (&(ring_buf->count), count_tmp, __ATOMIC_RELAXED);

//------------------------------------------------------------------------------
ret_val = pthread_mutex_unlock (&(ring_buf->tail_mtx));
assert (ret_val == 0); // if != 0, then it is really fatal situation
//------------------------------------------------------------------------------
// Unlock tail_mtx
//==============================================================================

//------------------------------------------------------------------------------
ret_val = pthread_mutex_unlock (&(ring_buf->head_mtx));
assert (ret_val == 0); // if != 0, then it is really fatal situation
//------------------------------------------------------------------------------
// Unlock head_mtx
//==============================================================================

*ring_buf_N_elems_buf_ptr = new_buf_for_N_elems;

return 0;


cleanup_from_memcpy:
        free (new_buf_for_buf_of_entries);
cleanup_from_buf_of_entries_calloc:
        free (new_buf_for_buf_of_elems);
cleanup_from_buf_of_elems_calloc:
        ret_val = pthread_mutex_unlock (&(ring_buf->tail_mtx));
        assert (ret_val == 0); // if != 0, then it is really fatal situation
cleanup_from_tail_mtx:
        ret_val = pthread_mutex_unlock (&(ring_buf->head_mtx));
        assert (ret_val == 0); // if != 0, then it is really fatal situation
cleanup_from_head_mtx:
        free (new_buf_for_N_elems);
        return -1;
}


int Ring_buf_delete_elems_after_get_N (ring_buf_N_elems_ptr* ring_buf_N_elems_buf_ptr)
{
if ((ring_buf_N_elems_buf_ptr && (*ring_buf_N_elems_buf_ptr)) == 0)
        {
        my_errno = wrong_args;
        return -1;
        }

free ((*ring_buf_N_elems_buf_ptr)->buf_of_elems[0].buf_for_entry);
(*ring_buf_N_elems_buf_ptr)->buf_of_elems[0].buf_for_entry = NULL;

free ((*ring_buf_N_elems_buf_ptr)->buf_of_elems);
(*ring_buf_N_elems_buf_ptr)->buf_of_elems = NULL;

free (*ring_buf_N_elems_buf_ptr);
(*ring_buf_N_elems_buf_ptr) = NULL;

return 0;
}







