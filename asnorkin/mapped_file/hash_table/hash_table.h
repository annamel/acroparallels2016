//*********************************************************
//********                                       **********
//********      Created by Alexander Snorkin     **********
//********              22.04.2016               **********
//********                                       **********
//*********************************************************
#ifndef HASH_TABLE_H
#define HASH_TABLE_H



#include "hash_funcs.h"



typedef struct item item_t;
typedef struct hash_table htable_t;



//  Item of the hash table structure
//  Contains the key for hash function,
//  value and next item pointer
struct item
{
    hkey_t key;
    hvalue_t value;
    item_t *next;
    item_t *prev;
};



//  The hash table structure
//  Contains the size of the table, hash function
//  pointer and the pointer on array of lists
struct hash_table
{
    int size;
    hfunc_t *hash_func;
    item_t **table;
};



//  This func initialize the hash table
//
//  - ARGUMENTS
//      size - size of the array of lists
//      hash_func - hash function pointer
//
//  - RETURNED VALUE
//      all is good => 0
//      hash_func pointer is NULL => EINVAL
//      can't allocate memory => ENOMEM
htable_t *ht_init(const int size, hfunc_t *hash_func);



//  This func deinitialize the hash table, remove
//  all items and the table
//
//  - ARGUMENTS
//      ht - the hash table pointer
//
//  - RETURNED VALUE
//      all is good => 0
//      hash_table pointer is NULL => EINVAL
int ht_deinit(htable_t *ht);



//  This func adds new item on the table by key.
//  Key have to be unique!!! If element with such key
//  already exist the element
//
//  - ARGUMENTS
//      ht - the hash table pointer
//
//  - RETURNED VALUE
//      all is good => 0
//      hash_table pointer is NULL => EINVAL
//      this key already exist => EKEYREJECTED
//      can't allocate the memory => ENOMEM
//      something else => -1
int ht_add_item(htable_t *ht, hkey_t key, hvalue_t value);



//  This func finds the element by the key
//
//  - ARGUMENTS
//      ht - the hash table pointer
//      key - the key for find
//      item - address of value of finded item will
//             write in this poiter
//
//  - RETURNED VALUE
//      all is good => 0
//      item with that key doesn't exist => ENOKEY
//      hash_table pointer is NULL => EINVAL
//      can't allocate memory for item => ENOMEM;
int ht_find_by_key(htable_t *ht, const hkey_t key, item_t **item);



//  This func finds the element by the key and value
//
//  - ARGUMENTS
//      ht - the hash table pointer
//      key - the key for find
//      value - the value for find
//      item - address of value of finded item will
//             write in this poiter
//
//  - RETURNED VALUE
//      all is good => 0
//      item with that key doesn't exist => ENOKEY
//      hash_table pointer is NULL => EINVAL
//      can't allocate memory for item => ENOMEM;
int ht_find_by_kav(htable_t *ht, hkey_t key, hvalue_t *value, item_t **item);



//  This func deleted item by the key if exist
//
//  - ARGUMENTS
//      ht - the hash table pointer
//      key - the key for delete
//
//  - RETURNED VALUE
//      item succesful deleted => 0
//      hash_table pointer is NULL => EINVAL
//      item doesn't exist => ENOKEY
//      list by key is empty => ENOKEY
int ht_del_item_by_key(htable_t *ht, hkey_t key);



//  This func deleted item by the key and value if exist
//
//  - ARGUMENTS
//      ht - the hash table pointer
//      key - the key for delete
//      value - the value for delete
//
//  - RETURNED VALUE
//      item succesful deleted => 0
//      hash_table pointer is NULL => EINVAL
//      item doesn't exist => ENOKEY
//      list by key is empty => ENOKEY
int ht_del_item_by_kav(htable_t *ht, hkey_t key, hvalue_t value);



//  This func prints the array of hash table and
//  information about every list in the array
//
//  - ARGUMENTS
//      ht - the hash table pointer
//
//  - RETURNED VALUE
//      no value
void ht_print_table(htable_t *ht);



//  This func calculate the number of items in the list on
//  particular array index
//
//  - ARGUMENTS
//      ht - the hash table pointer
//      index - the array index
//
//  - RETURNED VALUE
//      all is good => number of items by that index
//      hash_table pointer is NULL => EINVAL
//      index more than size of the table => EINVAL
int ht_number_of_items_in_index(htable_t *ht, int index);



//  This func prints the list by the index
//  Sequence numeration starts from nil
//
//  - ARGUMENTS
//      ht - the hash table pointer
//      index - the index of list for print
//
//  - RETURNED VALUE
//      all is good => 0
//      ht is NULL => EINVAL
//      index is out of array => EINVAL
//      something else => -1
int ht_print_list_in_index(htable_t *ht, int index);


#endif // HASH_TABLE_H
