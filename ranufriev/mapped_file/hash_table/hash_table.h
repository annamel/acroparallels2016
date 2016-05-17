#ifndef __HASH_TABLE_H_INCLUDED
#define __HASH_TABLE_H_INCLUDED

#include <stdio.h>


//==============================================================================
// Declaring of structures and types
//==============================================================================
typedef  void                hash_key_t;
typedef  void                hash_entry_t;
typedef  struct Hash_table*  hash_table_ptr;
typedef  struct Iterator*    iterator_ptr;

typedef  size_t (*hash_func_t)(const hash_key_t* hash_key, size_t hash_key_size, size_t hash_table_size);
//==============================================================================

//==============================================================================
// Declaring of API functions
//==============================================================================
/*
* Main points are:
*
* 1) RETURN VALUES:
* 1.1) If function returns 'int', then:
*         RETURN VALUE:  0 - if success
*                       -1 - otherwise, with my_errno indicating the error
*
* 1.2) If function returns some kind of 'pointer', then:
*         RETURN VALUE:  item_id (pointer) - if success
*                        NULL              - otherwise, with my_errno indicating the error
*
*
* 2) If pointer, passed as argument, doesn't have 'const' keyword than it will be
*       changed inside the function.
*       So like with std 'realloc' it is unsafe to write: 'my_iter = move_next (my_iter);'
*       because you may lose your ptr to my_iter if move_next fails.
*
*
* 3) MEMORY MANAGEMENT:
* 3.1) All information (like 'hash_key' and 'entry_to_add') that you give to hash_table are
*       duplicated and stored inside in their own memory blocks, so you could delete or
*       modify your own copy of this data as you want - it won't affect hash_table's ones.
*
* 3.2) But, if you get an iterator through any of succesfull calls to iterator functions
*       you should delete it with 'delete_iterator ()' function otherwise you will have
*       memory leak, cause hash_table doesn't manage iterators given to its elements.
*
* 3.3) Also, notice that 'get_key' and 'get_entry' functions return (if success) newly
*       allocated buffer where required thing stores, and YOU are responsible for
*       calling 'free ()' on this buffer, cause hash_table doen't manage this,
*       otherwise memory leak will happen.
*/

//------------------------------------------------------------------------------
// Functions dealing with hash_table
//------------------------------------------------------------------------------
// RETURN VALUE:  0 - if success
//               -1 - otherwise, with my_errno indicating the error
int Hash_table_construct (hash_table_ptr* hash_table, size_t hash_table_size, const hash_func_t hashing_func);
int Hash_table_destruct  (hash_table_ptr* hash_table);

int Hash_table_add_elem (hash_table_ptr hash_table,
                         const hash_key_t*   hash_key,     size_t hash_key_size,
                         const hash_entry_t* entry_to_add, size_t entry_size);
// deletes element and iterator, gived as a parameter too
int Hash_table_delete_elem (iterator_ptr* ptr_iter_to_del_elem);
//------------------------------------------------------------------------------
iterator_ptr find_elem (const hash_table_ptr hash_table, const hash_key_t* hash_key, size_t hash_key_size);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Functions dealing with iterators
//------------------------------------------------------------------------------
// RETURN VALUE: iterator_ptr - if success
//               NULL         - otherwise, with my_errno indicating the error
iterator_ptr get_iterator    (const hash_table_ptr hash_table);
int          delete_iterator (iterator_ptr*        usr_iter);
iterator_ptr dup_iterator    (const iterator_ptr   usr_iter);

// functions w/o const in params change the iter, so act as with realloc:
// avoid using this functions like this: 'my_iter = move_next (my_iter);'
iterator_ptr move_next (      iterator_ptr usr_iter);
iterator_ptr move_prev (      iterator_ptr usr_iter);
int          is_end    (const iterator_ptr usr_iter);         /* -1 - error;
                                                               *  0 - no, not end;
                                                               *  1 - yes, is end.
                                                               */

// RETURN VALUE:  pointer to required data's buffer - if success
//               -1                                 - otherwise, with my_errno indicating the error
// ATTENTION - funcs return pointer to new allocated buffer, that should be freed by YOU later
hash_key_t*   get_key   (const iterator_ptr usr_iter);
hash_entry_t* get_entry (const iterator_ptr usr_iter);
//------------------------------------------------------------------------------
hash_entry_t* get_real_entry (const iterator_ptr usr_iter);
//==============================================================================


#endif // __HASH_TABLE_H_INCLUDED

