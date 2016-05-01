//
//  pool_object.c
//  second_try_mf_lib
//
//  Created by IVAN MATVEEV on 22.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#include "pool_object.h"
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>

const size_t INIT_QUANTITY_ARRAYS = 10;
const size_t SIZE_FIRST_ARRAY = 10;
extern size_t mempagesize;

int _init_pool_object(PoolObject *pool, size_t size_table){
    pool->arrays.used = 0;
    pool->arrays.alloced_ptr = INIT_QUANTITY_ARRAYS;
    pool->arrays.arr = malloc(INIT_QUANTITY_ARRAYS * sizeof(Array_inode));
    if (pool->arrays.arr == NULL){
        errno = ENOMEM;
        return -1;
    }
    init_empty_list(&pool->free_list);
    if (_init_hash_table(&pool->table, size_table) < 0){
        free(pool->arrays.arr);
        errno = ENOMEM;
        return -1;
    }
    return 0;
}
PoolObject *init_pool_object(size_t size_table){
    PoolObject *pool = malloc(sizeof(PoolObject));
    if (pool == NULL){
        errno = ENOMEM;
        return NULL;
    }
    _init_pool_object(pool, size_table);
    return pool;
}
void _deinit_pool_object(PoolObject *pool){
    int i;
    for (i = 0; i < pool->arrays.used; i++)
        free(pool->arrays.arr[i].array);
    free(pool->arrays.arr);
}
void deinit_pool_object(PoolObject *pool){
    _deinit_pool_object(pool);
    free(pool);
}
int append_array(PoolObject *pool){
    if (pool == NULL){
        return -1;
    }
    size_t N = pool->arrays.used;
    if (N >= pool->arrays.alloced_ptr){
        pool->arrays.alloced_ptr *= 2;
        Array_inode *arr = realloc(pool->arrays.arr, pool->arrays.alloced_ptr);
        if (arr == NULL){
            errno = ENOMEM;
            return -1;
        } else
            pool->arrays.arr = arr;
    }
    size_t size = (N == 0) ? SIZE_FIRST_ARRAY : pool->arrays.arr[N-1].size * 2;
    pool->arrays.arr[N].size = size;
    pool->arrays.arr[N].array = malloc(size * sizeof(iNode));
    if (pool->arrays.arr[N].array == NULL){
        errno = ENOMEM;
        return -1;
    }
    pool->arrays.used++;
    int i;
    for (i = 0; i < size; i++)
        list_append(&pool->free_list, &pool->arrays.arr[N].array[i].node);
    return 0;
}
Node *pool_append(PoolObject *pool, Data data){
    if (pool == NULL)
        return NULL;
    if (list_is_empty(&pool->free_list))
        if (append_array(pool) < 0)
            return NULL;
    Node *node = list_get_first(&pool->free_list);
    node->value = data;
    hash_table_append(&pool->table, node);
    if (data.counter == 0){
        ilist_append(&pool->list_zero, node);
    }
    return node;
}
int data_free(Data data){
    if (munmap(data.ptr, data.size_in_pages*mempagesize) < 0)
        return -1;
    return 0;
}
int pool_free_space(PoolObject *pool){
    ssize_t size_list_zero = ilist_size(&pool->list_zero);
    if (size_list_zero < 0)
        return -1;
    int i;
    iNode *inode;
    for (i = 0; i < size_list_zero/2; i++){
        inode = ilist_get_last(&pool->list_zero);
        if (inode == NULL)
            return -1;
        hash_table_remove(&pool->table, &inode->node);
        data_free(inode->node.value);
    }
    return 0;
}
Node *pool_find(PoolObject *pool, size_t number_first_page, int (*check)(Data)){
    return hash_tablde_find(&pool->table, number_first_page, check);
}



