//
//  hash_table.c
//  mf_project
//
//  Created by IVAN MATVEEV on 16.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//


#include "hash_table.h"

unsigned int BPhash(int value)
{
    unsigned int hash = 0;
    unsigned int i    = 0;
    unsigned char *str = (unsigned char *)&value;
    for(i = 0; i < 4; str++, i++) {
        hash = hash << 7 ^ (*str);
    }
    return hash;
}

unsigned int NewHashFunc(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}

int key(Data value) {
    return (int)value.number_first_page;
}
void removeHashTable(HashTable *table) {
    if (table == NULL)
        return;
    int i;
    for (i = 0; i < table->size; i++)
        clearList(&(table->array[i]));
    free(table->array);
    free(table);
    table = NULL;
}
void displayHashTable(HashTable *table) {
    if (table == NULL)
        return;
    printf("displayHashTable\n");
    int i;
    for (i = 0; i < table->size; i++) {
        printf("hash = %d\n", i);
        displayList(&table->array[i]);
    }
}
HashTable *initHashTable(size_t size) {
    HashTable *table = malloc(sizeof(HashTable));
    error_with_malloc(table == NULL, NULL);
    table->size = size;
    table->array = malloc(sizeof(HeaderList)*size);
    if (table->array == NULL){
        free(table);
        errno = ENOMEM;
        return NULL;
    }
    HeaderList *empty_list = emptyList();
    if (empty_list == NULL)
        return NULL;
    int i;
    for (i = 0; i < size; i++)
        table->array[i] = *empty_list;
    table->hashFunc = NewHashFunc;
    return table;
}
int appendInHashTable(HashTable *table, Data value) {
    int hash = table->hashFunc(key(value)) % table->size;
    return appendNode(&table->array[hash], value);
}
// return value: 0 if all ok
//              negative value if have error or no find val
int removeFromHashTable(HashTable *table, size_t number_first_page, size_t size_in_page) {
    int hash = table->hashFunc((int)number_first_page) % table->size;
    return removeNode(&table->array[hash], number_first_page, size_in_page, 1);
}
Data *getFromHashTable(HashTable *table, size_t number_first_page, size_t size_in_page) {
    int hash = table->hashFunc((int)number_first_page) % table->size;
    return getNode(&table->array[hash], number_first_page, size_in_page);
}





