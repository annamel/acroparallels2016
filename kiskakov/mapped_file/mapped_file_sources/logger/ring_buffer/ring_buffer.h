#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h> // for uint32_t

#include "ret_code.h"

#define RING_BUFFER_SIZE 1024



typedef struct ring_buffer ring_buffer_t;
typedef void   ring_buffer_elem_t;

ret_code_t ring_buffer_construct(ring_buffer_t ** , uint32_t);
ret_code_t ring_buffer_destruct(ring_buffer_t **);

ret_code_t ring_buffer_put (ring_buffer_t * , const ring_buffer_elem_t *, double *);
ret_code_t ring_buffer_get (ring_buffer_t * , ring_buffer_elem_t * );
ret_code_t ring_buffer_getn(ring_buffer_t * , ring_buffer_elem_t ** , uint32_t, uint32_t *);

ret_code_t ring_buffer_print(ring_buffer_t *, void (*) (const ring_buffer_elem_t *));


#endif // RING_BUFFER_H
