#include "chunk_manager.h"
#include "logger.h"
#ifndef hash_table
#define hash_table

typedef off_t value_key_t;
typedef uint32_t hash_key_t;
typedef struct element list_element;

struct element {
	list_element* next;
	list_element* prev;
	chunk_t* data;
};

typedef struct hash_table {
    unsigned int size;
    unsigned is_initialized;
    list_element** table;
} hash_table_t;


hash_key_t hash_func (value_key_t key);
hash_table_t* hash_table_init(unsigned int size); 
int hash_table_deinit(hash_table_t* h_table);
int add_element(chunk_t* chunk); // hash_table_t/////
int remove_element(hash_table_t* h_table, off_t index, off_t length);
int find_value(hash_table_t* h_table, off_t index, off_t length);
chunk_t* take_value_ptr(hash_table_t* h_table, off_t index, off_t length);
chunk_t* find_by_index(hash_table_t* h_table, off_t index);
chunk_t* find_in_range(hash_table_t* h_table, off_t offset, size_t size);



//void printElementsInIndex(list_element* ptr, int index);
//void print_hash_table(hash_table_t* h_table);

#endif
