#ifndef hash_table
#define hash_table

#include "chunkmanager.h"
#include "common_types.h"
#include "logger.h"
#include <stdint.h>

typedef off_t value_key_t;
typedef uint32_t hash_key_t;
typedef chunk_t* data_t;

hash_table_t* hash_table_init(unsigned int size);
int hash_table_deinit(hash_table_t* h_table);
int remove_element(hash_table_t* h_table, off_t index, off_t length);
int find_value(hash_table_t* h_table, off_t index, off_t length);
int add_element(data_t chunk);
data_t take_value_ptr(hash_table_t* h_table, off_t index, off_t length);
data_t find_by_index(hash_table_t* h_table, off_t index);
data_t find_in_range(ch_pool_t* ch_pool, off_t offset, size_t size);
hash_key_t hash_func (value_key_t key);

#endif

