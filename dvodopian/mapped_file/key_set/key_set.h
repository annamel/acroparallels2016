//
// Created by voddan on 22/04/16.
//

#ifndef FILE_MAPPER_PROJECT_ORDERED_SET_H
#define FILE_MAPPER_PROJECT_ORDERED_SET_H


#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct key_set key_set;

typedef uint16_t set_key_t;

key_set * set_alloc();

void set_free(key_set * sp);

/* set NULL to remove */
void set_set(key_set * sp, set_key_t key, void * item);

/* returns NULL if nothing found */
void * set_get(key_set * sp, set_key_t key);

bool set_contains(key_set* sp, set_key_t key);


#ifdef __cplusplus
}
#endif

#endif //FILE_MAPPER_PROJECT_ORDERED_SET_H
