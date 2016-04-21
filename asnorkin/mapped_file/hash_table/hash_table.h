//*********************************************************
//********                                       **********
//********   Created by asnorkin on 22.03.2016.  **********
//********                                       **********
//*********************************************************
#ifndef HASH_TABLE_HASH_TABLE_H
#define HASH_TABLE_HASH_TABLE_H


#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>


#ifdef _MSC_VER
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif


// default hash function values
#define HASH_CONST_1 0x01000193 //   16777619
#define HASH_CONST_2 0x811C9DC5 // 2166136261


typedef void *value_t;
typedef off_t hash_key_t;

typedef hash_key_t hash_func_t(const hash_key_t,  uint32_t);


//  Item of the hash table structure
//  Contains the key for hash function,
//  value and next item pointer
typedef struct item_t
{
    hash_key_t key;
    value_t value;
    struct item_t *next;
} item_t;


//  The hash table structure
//  Contains the size of the table, hash function
//  pointer and the pointer on array of lists
typedef struct hash_table_t
{
    int size;
    hash_func_t *hash_func;
    item_t **table;
} hash_table_t;



//  This func initialize the hash table
//
//  - ARGUMENTS
//      size - size of the array of lists
//      hash_func - hash function pointer
//
//  - RETURNED VALUE
//      all is good => hash table pointer
//      calloc works bad => NULL
hash_table_t *hash_table_init(const int size,
                              hash_func_t *hash_func);



//  This func deinitialize the hash table, remove
//  all items and the table
//
//  - ARGUMENTS
//      hash_table - the hash table pointer
//
//  - RETURNED VALUE
//      all is good => 0
//      hash_table pointer is NULL => EINVAL
int hash_table_deinit(hash_table_t *hash_table);



//  This func adds new item on the table by key.
//  Key have to be unique!!! If element with such key
//  already exist the element
//
//  - ARGUMENTS
//      hash_table - the hash table pointer
//
//
//  - RETURNED VALUE
//      all is good => pointer on added item
//      this key already exist => pointer on existing
//                                item with such key
//      hash_table pointer is NULL => NULL
//      calloc worked bad => NULL
//      something else => NULL
int add_item(hash_table_t *hash_table,
                 hash_key_t new_key, value_t new_value);



//  This func prints the array of hash table and
//  information about every list in the array
//
//  - ARGUMENTS
//      hash_table - the hash table pointer
//
//  - RETURNED VALUE
//      no value
void print_table(hash_table_t *hash_table);



//  This func prints the list by the index
//  Sequence numeration starts from nil
//
//  - ARGUMENTS
//      hash_table - the hash table pointer
//      index - the index of list for print
//
//  - RETURNED VALUE
//      no value
void print_list_in_index(hash_table_t *hash_table,
                         int index);



//  This func calculate the number of items in the list on
//  particular array index
//
//  - ARGUMENTS
//      hash_table - the hash table pointer
//      index - the array index
//
//  - RETURNED VALUE
//      all is good => number of items by that index
//      hash_table pointer is NULL => -1
//      index more than size of the table => -2
int number_of_items_in_index(hash_table_t *hash_table,
                             int index);



//  This func finds the element by the key
//
//  - ARGUMENTS
//      hash_table - the hash table pointer
//      key - the key for find
//      value - address of value of finded item will
//              write in this poiter
//
//  - RETURNED VALUE
//      all is good => finded item pointer
//      error occured => errno code
int find_value(hash_table_t *hash_table,
                   const hash_key_t key, value_t *value);



//  This func deleted item by the key if exist
//
//  - ARGUMENTS
//      hash_table - the hash table pointer
//      key - the key for delete
//
//  - RETURNED VALUE
//      item succesful deleted => 0
//      hash_table pointer is NULL => EINVAL
//      item doesn't exist => ENOKEY
//      list by key is empty => -ENOKEY
int delete_item(hash_table_t *hash_table, hash_key_t key);


//*********************************************************
//**********                                     **********
//**********           HASH FUNCTIONS            **********
//**********                                     **********
//*********************************************************
//  This func generates uint32 hash key by uint32 key
//
//  - ARGUMENTS
//      key - unit32 key for generate the hash key
//      hash - const for generating
//
//  - RETURNED VALUE
//      all is good => hash key
hash_key_t hash_fnv1a(const hash_key_t key, uint32_t hash);

//  This func calculates the uint32 hash key by one byte
//  Assistance func for hash_fnv1a
uint32_t fnv1a(unsigned char oneByte, uint32_t hash);

#endif //HASH_TABLE_HASH_TABLE_H
