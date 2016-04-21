#ifndef hash_table_h
#define hash_table_h

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>


typedef uint32_t value_key_t;
typedef unsigned long long int value_t;
typedef uint32_t hash_key_t;

typedef struct list_element {
	struct list_element* next;
	struct list_element* prev;
	size_t number_first_page;
    size_t size_in_pages;
    unsigned int counter;
    void *ptr;
	value_key_t key;
	value_t data;
} list_element;

typedef struct hash_table {
	unsigned int size;
	list_element** table;
} hash_table_t;

hash_key_t hash_func (value_key_t key);
hash_table_t* hash_table_init(const unsigned int size);
int deinit (hash_table_t* table);
int add_element(hash_table_t* table, value_key_t key, value_t* value);
int remove_element(hash_table_t* table, value_key_t key, value_t* value); //Наверно придется убрать Value, ну по смыслу чтобы было нормально, то есть по ключу удалить значение.
int find_value(hash_table_t* table, value_key_t key, value_t* value);
int numberOfItemsInIndex(hash_table_t* table, int index);
void printElementsInIndex(list_element* ptr, int index);
void print_hash_table(hash_table_t* table); 

#endif
