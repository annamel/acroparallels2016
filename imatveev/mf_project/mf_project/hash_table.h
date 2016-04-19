//
//  hash_table.h
//  mf_project
//
//  Created by IVAN MATVEEV on 15.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#ifndef hash_table_h
#define hash_table_h

#include <errno.h>
#include <malloc/malloc.h>
#include <stdio.h>
#include "list.h"
#include "best_error_define_ever.h"

typedef struct HashTable {
    HeaderList *array;
    size_t size;
    int (*hashFunc)(int);
} HashTable;

unsigned int NewHashFunc(unsigned int x);
unsigned int BPhash(int value);
int key(Data value);
HashTable *initHashTable(size_t size);
void removeHashTable(HashTable *table);
void displayHashTable(HashTable *table);
int appendInHashTable(HashTable *table, Data value);
int removeFromHashTable(HashTable *table, size_t number_first_page, size_t size_in_page);
Data *getFromHashTable(HashTable *table, size_t number_first_page, size_t size_in_page);

#endif /* hash_table_h */
