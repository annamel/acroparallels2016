#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "inverted_index.h"

struct Ii_manager* init_ii_m(){

    size_t block_size = 112;

    if(sizeof(void*) == 4) {
        block_size = 224;
    }

    size_t size = block_size * sizeof(struct Ii_element) + sizeof(struct Block) + sizeof(struct Ii_manager);
    struct Ii_manager* manager = (struct Ii_manager*)calloc(1, size);
    if(!manager)
        return 0;


    manager -> data = (struct Block**)calloc(sizeof(void*), INIT_BLOCK_COUNT);
    if(!manager -> data) {
        free(manager);
        return 0;
    }

    manager -> block_size = block_size;
    manager -> data[manager -> block_count++] = (struct Block*)((void*)manager + sizeof(struct Ii_manager));
    manager -> block_max = INIT_BLOCK_COUNT;

    return manager;
}

ssize_t destruct_ii_m(struct Ii_manager* manager) {
    if(!manager)
        return 0;

    for(int i = 1; i < manager -> block_count; i++)
        free(manager -> data[i]);

    free(manager);

    return 0;
}

struct Ii_element* allocate_element(struct Ii_manager* manager, void* item, size_t pos_inside){
    if(!manager)
        return 0;

    for(int i = 0; i < manager -> block_count; i++) {

        if(manager -> data[i] -> count == manager -> block_size)
            continue;

        struct Ii_element* tmp = manager -> data[i] -> data;

        for(int j = 0; j < manager -> block_size; j++) {
            if(tmp[j].state == EMPTYII) {

                tmp[j].item = item;
                tmp[j].pos_inside = pos_inside;


                manager -> data[i] -> count++;

                tmp[j].state = READYII;

                return &tmp[j];
            }

        }
    }

    //actions if all blocks are full

    void* free_candidate = 0;

    if(manager -> block_count == manager -> block_max) {

        struct Block** tmp_data = (struct Block**)calloc(manager -> block_max * 2, sizeof(void*));
        if(!tmp_data)
            return 0;
        memcpy(tmp_data, manager -> data, sizeof(void*) * manager -> block_count);

        free_candidate = manager -> data;
        manager -> data = tmp_data;
        manager -> block_max *= 2;
    }

    size_t index = manager -> block_count;
    struct Block* tmp = (struct Block*)calloc(1, sizeof(struct Block) + sizeof(struct Ii_element) * manager -> block_size);
    if(!tmp)
        return (void*)-1;

    manager -> data[index] = tmp;
    manager -> block_count++;

    free(free_candidate); //Actually this can be a real problem if someone is handling previous manager -> data

    return allocate_element(manager, item, pos_inside);
}


ssize_t destruct_element(struct Ii_manager* manager, struct Ii_element* item) {
    if(!manager || !item)
        return -1;

    for(int i = 0; i < manager -> block_count; i++) {
        size_t bottom = (size_t)item - (size_t)(manager -> data[i] -> data);
        size_t top = (size_t)(manager -> data[i] -> data + manager -> block_size) - (size_t)item;

        if(bottom <= manager -> block_size * sizeof(struct Ii_element) && top <= manager -> block_size * sizeof(struct Ii_element)) {
            manager -> data[i] -> count--;
            item -> state = EMPTYII;


            return 0;
        }
    }

    return -1;
}


struct Inverted_index* init_ii(size_t size) {

    size_t alloc_size = sizeof(void*) * size + sizeof(struct Inverted_index);

    struct Inverted_index* ii = (struct Inverted_index*)calloc(1, alloc_size);
    if(!ii)
        return (void*)-1;

    ii -> manager = init_ii_m();
    if(ii -> manager == (void*)-1) {
        free(ii);
        return (void*)-1;
    }

    return ii;
}

ssize_t destruct_ii(struct Inverted_index* ii) {
    if(!ii)
        return 0;

    destruct_ii_m(ii -> manager);
    free(ii);

    return 0;
}

ssize_t add_item(struct Inverted_index* ii, void* item, size_t start, size_t until){
    if(!ii)
        return -1;

    for(size_t i = start; i < until; i++) {

        struct Ii_element* tmp = allocate_element(ii -> manager, item, i - start);
        if(tmp == (void*)-1)
            return i - start;

        tmp -> next = 0;

        struct Ii_element* place = ii -> data[i];
        if(!place) {
            ii -> data[i] = tmp;
            continue;
        }

        while(place -> next)
            place = place -> next;

        place -> next = tmp;
    }

    return until - start;
}

ssize_t delete_item(struct Inverted_index* ii, void* item, size_t start, size_t until) {
    if(!ii)
        return 0;

    for(size_t i = start; i <= until; i++) {

        struct Ii_element* place = ii -> data[i];
        if(place -> item == item) {
            ii -> data[i] = place -> next;
            destruct_element(ii -> manager, place);

            continue;
        }

        while(place -> next && place -> next -> item != item)
            place = place -> next;

        if(!place -> next)
            return until - start;

        place -> next = place -> next -> next;
        destruct_element(ii -> manager, place -> next);

    }

    return until - start;
}
