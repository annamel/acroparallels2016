#ifndef __RING_BUF_H_INCLUDED
#define __RING_BUF_H_INCLUDED

#include <stddef.h>
// 'ring_buf' supports thread-safe implementation of 'my_errno' (TLS)
#include "../error/error.h"

typedef  void               ring_buf_entry_t;
typedef  ring_buf_entry_t*  ring_buf_entry_ptr;

typedef struct Ring_buf_elem
        {
        size_t            size_of_entry;
        ring_buf_entry_t* buf_for_entry;
        } ring_buf_elem_t;

typedef ring_buf_elem_t* ring_buf_elem_ptr;


typedef struct Ring_buf_N_elems
        {
        size_t           numb_of_elems;
        ring_buf_elem_t* buf_of_elems;
        } ring_buf_N_elems_t;

typedef ring_buf_N_elems_t* ring_buf_N_elems_ptr;

typedef  struct Ring_buf*   ring_buf_ptr;

// WARNING: 'construct' and 'destruct' is not thread-safe!

// 'ring_buf_size' is set in 'Ring_buf_construct' and cannot be changed later
//
// If func returns 'mutex_destr_err' probably smth is really wrong and you can 'assert' this state,
//      otherwise to avoid memory leak you need to call 'destruct' until it returns the success
int Ring_buf_construct (ring_buf_ptr* ring_buf, size_t ring_buf_size);
// This function should be called only when all work with 'ring_buf' is ended,
//      e.g. all threads were joined and so on, because it is not thread-safe!
// But if error has occurred, it is safe to call this func one more time.
//
// If func returns 'mutex_destr_err' probably you are calling this func at a wrong time
//      and you can 'assert' this state,
//      otherwise to avoid memory leak you need to call 'destruct' until it returns the success
int Ring_buf_destruct  (ring_buf_ptr* ring_buf);

// all info given to 'ring_buf' is duplicated and stored inside 'ring_buf', so you could modify/delete
//      your copies w/o side effects
//
// RETURN_VALUE:  positive integer in range of [0..100) indicating ring_buf occupancy percentage - if success
//                                                                                            -1 - otherwise
int Ring_buf_put (ring_buf_ptr ring_buf, const ring_buf_entry_t* ring_buf_entry, size_t size_of_ring_buf_entry);
// Func allocate a buffer able to store 1 object of type 'ring_buf_elem_t'
//      and give a ptr to this buffer as 'ring_buf_elem_buf_ptr'
int Ring_buf_get (const ring_buf_ptr ring_buf, ring_buf_elem_ptr* ring_buf_elem_buf_ptr);

int Ring_buf_delete_elem_after_get (ring_buf_elem_ptr* ring_buf_elem_buffer);

// Func allocate an array of ptrs and a buffer for each ptr so that each buffer
//      is able to store 1 object of type 'ring_buf_entry_t'
//      and give a ptr to this array as 'ring_buf_entry_buffer'
//
// RETURN_VALUE:   0 - if success
//                -1 - otherwise
int Ring_buf_get_N_elems (ring_buf_ptr ring_buf, ring_buf_N_elems_ptr* ring_buf_N_elems_buf_ptr, size_t N);

int Ring_buf_delete_elems_after_get_N (ring_buf_N_elems_ptr* ring_buf_N_elems_buf_ptr);




#endif // __RING_BUF_H_INCLUDED
