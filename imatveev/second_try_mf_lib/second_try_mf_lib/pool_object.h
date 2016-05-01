//
//  pool_object.h
//  second_try_mf_lib
//
//  Created by IVAN MATVEEV on 22.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#ifndef pool_object_h
#define pool_object_h

#include <stdio.h>
#include "list.h"
#include "i_list.h"
#include "hash_table.h"

typedef struct Array_inode{
    iNode *array;
    size_t size;
} Array_inode;

typedef struct Arrays{
    Array_inode *arr;
    size_t alloced_ptr;
    size_t used;
}Arrays;

typedef struct pool_object{
    Arrays arrays;
    HashTable table;
    List free_list;
    iList list_zero;
} PoolObject;

int data_free(Data data);
PoolObject *init_pool_object(size_t size_table);
int _init_pool_object(PoolObject *pool, size_t size_table);
void  deinit_pool_object(PoolObject *pool);
void _deinit_pool_object(PoolObject *pool);
int append_array(PoolObject *pool);
Node *pool_append(PoolObject *pool, Data data);
Node *pool_find(PoolObject *pool, size_t number_first_page, int (*check)(Data));
// retunr -1 if can't free anything, 0 if success
int pool_free_space(PoolObject *pool);

#endif /* pool_object_h */
