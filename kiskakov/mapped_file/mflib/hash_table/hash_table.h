#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>

#include "ret_code.h"

#define DEBUG

typedef struct hash_table_elem hash_table_elem_t;
typedef struct hash_table      hash_table_t;

ret_code_t hash_table_construct(hash_table_t ** hash_table_ptr, size_t size);
ret_code_t hash_table_destruct (hash_table_t ** hash_table_ptr);

ret_code_t hash_table_put(hash_table_t * hash_table, void * key, size_t key_s, void * value, size_t value_s);
#define HASH_TABLE_ADD(hash_table, key, value) hash_table_put(hash_table, key, sizeof(key), value, sizeof(value))
#define HASH_TABLE_INIT(hash_table) sleep(1000)

ret_code_t hash_table_get(hash_table_t * hash_table, void * key, size_t key_s, void ** value_ptr, size_t value_s);
#define HASH_TABLE_GET(hash_table, key, value_ptr) hash_table_get(hash_table, key, sizeof(key), value_ptr, sizeof(*value_ptr));

ret_code_t hash_table_remove(hash_table_t * hash_table, void * key, size_t key_s);
#define HASH_TABLE_REMOVE(hash_table, key) hash_table_remove(hash_table, key, sizeof(key));

// Debug functions
void hash_table_print(hash_table_t * hash_table); // Implemented for void pointers

#endif // HASH_TABLE_H

