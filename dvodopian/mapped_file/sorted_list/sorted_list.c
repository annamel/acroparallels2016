//
// Created by voddan on 22/04/16.
//

#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "sorted_list.h"
#include "../chunk_manager/chunk_manager.h"

#define ROW_ARR_SIZE_START  (5)
#define ROW_ARR_SIZE_FACTOR (1.5)


typedef struct row {
    int key;
    int count;

    int arr_size;
    void** arr;

    struct row* next_row; // NULL
    struct row* prev_row; // NULL  // todo: remove
} row;

struct sorted_list {
    comparator_t* func;
    int count;
    row* first_row;
};

/*
 * ROW
 * */

/* func(item) must equal to rp->key */
void row_add(row * rp, row * prev_rp, void * item) {
    assert(rp->count <= rp->arr_size);

    assert(prev_rp == rp->prev_row);  // TEST

    if(rp->count == rp->arr_size) {  // approximately
        int size = (int) (rp->arr_size * ROW_ARR_SIZE_FACTOR) + 1;

        void ** arr = calloc(sizeof(void*), (size_t) size);
        int j = 0;

        for(int i = 0; i < rp->arr_size; i++)
            if(rp->arr[i]) {
                arr[j] = rp->arr[i];
                j += 1;
            }

        arr[j] = item;

        rp->arr = arr;
        rp->arr_size = size;
    } else {
        for (int i = 0; i < rp->arr_size; ++i)
            if(!rp->arr[i]) {
                rp->arr[i] = item;
                break;
            }
    }

    rp->count += 1;
}

void row_del(row * rp, row * prev_rp, int index) {
//    assert(0 <= index < rp->count);
    assert(rp->count <= rp->arr_size);

    assert(prev_rp == rp->prev_row);  // TEST


    if(index < 0 || index >= rp->count)
        return;

    int count_down = index;

    for (int i = 0; i < rp->arr_size; ++i)
        if(rp->arr[i]) {
            if(count_down > 0)
                count_down -= 1;
            else {
                rp->arr[i] = NULL;
                rp->count -= 1;

                // TEST : freeing the raw??
                return;
            }
        }

}

row* row_alloc(int key) {
    row* rp = malloc(sizeof(row));
    rp->key = key;
    rp->count = 0;

    rp->arr_size = ROW_ARR_SIZE_START;
    rp->arr = calloc(sizeof(void*), (size_t) rp->arr_size);

    rp->next_row = NULL;
    rp->prev_row = NULL;

    return rp;
}

void row_free(row * rp) {
    free(rp->arr);
    free(rp);
}


/* 
 * Public API
 * */

sorted_list * list_alloc(comparator_t* func) {
    sorted_list* lp = malloc(sizeof(sorted_list));
    lp->func = func;
    lp->count = 0;
    lp->first_row = NULL;
    return lp;
}

void list_free(sorted_list * lp) {
    row* r = lp->first_row;
    
    while(r) {
        row* nr = r->next_row;
        row_free(r);
        r = nr;
    }
    
    free(lp);
}


void list_add(sorted_list * lp, void * item) {
    lp->count += 1;
    
    const int key = lp->func(item);

    row* previous = NULL;
    for(row* r = lp->first_row; r; r = r->next_row) {
        if(key == r->key) {
            row_add(r, previous, item);
            return;
        } else if (r->key > key) {
            row* prev = previous;
            row* next = r;
            
            row* here = row_alloc(key);
            here->next_row = next;
            here->prev_row = prev;

            next->prev_row = here;

            if(prev) 
                prev->next_row = here;
            else
                lp->first_row = here;
            
            row_add(here, previous, item);
            return;
        } // else go on
        
        previous = r;
    }

    // add a row to the very end

    row * here = row_alloc(key);
    here->prev_row = previous;
    row_add(here, previous, item);

    if(previous) {
        previous->next_row = here;
    } else {
        lp->first_row = here;
    }
}

void list_del(sorted_list * lp, const int index) {
//    assert(0 <= index < lp->count);
    if(index < 0 || index >= lp->count)
        return;

    int count_down = index;
    row* previous = NULL;

    for(row* r = lp->first_row; r; r = r->next_row) {
        int rc = r->count;

        if(count_down < rc) {
            row_del(r, previous, count_down);
            lp->count -= 1;
            return;
        } else
            count_down -= rc;

        previous = r;
    }

    assert("WTF! ; r == NULL; index not found" && index);
}


void * list_get(sorted_list * lp, int index) {
//    assert(0 <= index < lp->count);
    if(index < 0 || index >= lp->count)
        return NULL;

    int count_down = index;

    for(row* r = lp->first_row; r; r = r->next_row) {
        const int rc = r->count;
        assert(rc >= 0);

        if(count_down >= rc)
            count_down -= rc;  // next row
        else {
            for(int i = 0; i < r->arr_size; i++ ) {
                if(!r->arr[i]) continue;

                if(count_down == 0)
                    return r->arr[i];
                else
                    count_down -= 1;
            }

            assert("WTF! ; index not found in row" && index);

        }


        assert(count_down >= 0);
    }

    assert("WTF! ; r == NULL; index not found" && index);
}

int list_size(sorted_list * lp) {
    return lp->count;
}


int list_search(sorted_list* lp, void * item) {
    int index = 0;

    for(row* r = lp->first_row; r; r = r->next_row) {
        int row_index = 0;  // optimization

        for (int i = 0; i < r->arr_size; ++i) {
            void * e = r->arr[i];
            if(e) {
                if(e == item)
                    return index;

                index += 1;
                row_index += 1;

            }

            if(row_index >= r->count)
                break;
        }
    }

    return -1;

}

void list_remove(sorted_list * lp, void * item) {
    for(row* r = lp->first_row; r; r = r->next_row) {
        int row_index = 0;

        row* previous = NULL;
        for (int i = 0; i < r->arr_size; ++i) {
            void * e = r->arr[i];
            if(e) {
                if(e == item) {
                    row_del(r, previous, row_index);
                    lp->count -= 1;
                    return;
                }

                row_index += 1;
            }

            if(row_index >= r->count)
                break;

            previous = r;
        }
    }
}


