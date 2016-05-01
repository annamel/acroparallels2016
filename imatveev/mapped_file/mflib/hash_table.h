//
//  hash_table.h
//  second_try_mf_lib
//
//  Created by IVAN MATVEEV on 23.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#ifndef hash_table_h
#define hash_table_h

#include <stdio.h>
#include "list.h"


typedef struct HashTable {
    List *array;
    size_t size;
    size_t (*hash_func)(size_t);
} HashTable;

size_t new_hash_func(size_t x);
size_t BPhash(size_t value);
size_t key(Node *node);
HashTable *init_hash_table(size_t size);
int _init_hash_table(HashTable *table, size_t size);
// RETURN VALUE: 0 - success, -1 - failed
int hash_table_append(HashTable *table, Node *new_node);
int hash_table_remove(HashTable *table, Node *node);
Node *hash_tablde_find(HashTable *table, size_t number_first_page, int (*check)(Data));

#endif /* hash_table_h */
