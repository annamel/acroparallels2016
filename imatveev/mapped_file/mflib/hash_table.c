//
//  hash_table.c
//  second_try_mf_lib
//
//  Created by IVAN MATVEEV on 23.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#include "hash_table.h"
#include <stdlib.h>
#include <errno.h>

size_t BPhash(size_t value)
{
    unsigned int hash = 0;
    unsigned int i    = 0;
    unsigned char *str = (unsigned char *)&value;
    for(i = 0; i < 4; str++, i++) {
        hash = hash << 7 ^ (*str);
    }
    return hash;
}

size_t new_hash_func(size_t x){
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}
HashTable *init_hash_table(size_t size){
    HashTable *table = malloc(sizeof(HashTable));
    if (table == NULL){
        errno = ENOMEM;
        return NULL;
    }
    table->array = malloc(size * sizeof(List));
    if (table->array == NULL){
        free(table);
        errno = ENOMEM;
        return NULL;
    }
    int i;
    for (i = 0; i < size; i++)
        init_empty_list(&table->array[i]);
    table->size = size;
    table->hash_func = new_hash_func;
    return table;
}
int _init_hash_table(HashTable *table, size_t size){
    table->array = malloc(size * sizeof(List));
    if (table->array == NULL){
        errno = ENOMEM;
        return -1;
    }
    int i;
    for (i = 0; i < size; i++)
        init_empty_list(&table->array[i]);
    table->size = size;
    table->hash_func = new_hash_func;
    return 0;
}
size_t key(Node *node){
    return node->value.number_first_page;
}
int hash_table_append(HashTable *table, Node *new_node){
    size_t hash = table->hash_func(key(new_node)) % table->size;
    return list_append(&table->array[hash], new_node);
}
int hash_table_remove(HashTable *table, Node *node){
    size_t hash = table->hash_func(key(node)) % table->size;
    return list_remove(&table->array[hash], node);
}
Node *hash_tablde_find(HashTable *table, size_t number_first_page, int (*check)(Data)){
    size_t hash = table->hash_func(number_first_page) % table->size;
    return list_find(&table->array[hash], check);
}






