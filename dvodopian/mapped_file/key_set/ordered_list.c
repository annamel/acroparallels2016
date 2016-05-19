//
// Created by voddan on 22/04/16.
//

#include <stdlib.h>
#include <memory.h>
#include "key_set.h"


/* FUN OPTIMIZATION */

#define BIT_SPLIT (8)

#define ROW_S (1 << BIT_SPLIT)
#define TAB_S (UINT16_MAX >> BIT_SPLIT)

/*
 * HASH TABLE STRUCTURE
 * */
 
typedef void* row_t[ROW_S];

struct key_set {
    row_t* table[TAB_S];
};

/* KEY MANIPULATION */

uint16_t key_tab_index(set_key_t key) {
    return key >> BIT_SPLIT;
}

uint16_t key_row_index(set_key_t key) {
    size_t shift = sizeof(uint16_t) * 8 - BIT_SPLIT;
    return (key << shift) >> shift;
}

/*
 * PUBLIC API
 * */

key_set* set_alloc() {
    key_set* sp = malloc(sizeof(key_set));
    memset(sp->table, 0, sizeof(sp->table));

    return sp;
}

/* does not free the items */
void set_free(key_set* sp) {
    for (int i = 0; i < TAB_S; ++i) {
        free(sp->table[i]);
    }
    
    free(sp);
}

/* set NULL to remove */
void set_set(key_set* sp, set_key_t key, void* item) {
    row_t * rp = sp->table[key_tab_index(key)];
    if(rp) {
        *rp[key_row_index(key)] = item;

        if(!item) {
            for (int i = 0; i < ROW_S; ++i) {
                if(*rp[i]) return;
            }

            free(rp);
            sp->table[key_tab_index(key)] = NULL;
        }
    } else {
        if(!item) return;

        rp = malloc(sizeof(row_t));
        memset(rp, 0, sizeof(row_t));
        *rp[key_row_index(key)] = item;
        sp->table[key_tab_index(key)] = rp;
    }
}

/* returns NULL if nothing found */
void * set_get(key_set* sp, set_key_t key) {
    row_t * rp = sp->table[key_tab_index(key)];
    return !rp ? NULL : *rp[key_row_index(key)];
}

bool set_contains(key_set* sp, set_key_t key) {
    row_t * rp = sp->table[key_tab_index(key)];
    return !rp ? false : *rp[key_row_index(key)] != NULL;
}