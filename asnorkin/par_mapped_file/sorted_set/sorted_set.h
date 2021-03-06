#ifndef SORTED_SET_H
#define SORTED_SET_H
//  This structure contains chunks in the order of increasing by indexes
//  and in the order of decreasing by lengths for every index
//      Example:    index   len
//                  1       5
//                  1       3
//                  6       0
//                  124     7
//                  124     4
//                  124     3
//                  ..      ..


#include "../chunk_manager/chunk_manager.h"
#include "../typedefs.h"



//  This func initializes a new sorted set
//  - RETURNED VALUE:
//      all is good => new sorted set pointer
//      calloc has failed => NULL
sset_t *ss_init();



//  This func deinits the sorted set
//  - ARGUMENTS
//      sset - sorted set pointer
//
//  - RETURNED VALUE
//
int ss_deinit(sset_t *sset);



//  This func adds new element in set
//  - ARGUMENTS
//      sset - sorted set pointer
//      value - new value
//
//  - RETURNED VALUE
//      all is good => 0
//      set pointer is NULL => EINVAL
//      calloc has failed => -1(see errno)
//      such value has already exist => EKEYREJECTED
int ss_add(sset_t *sset, chunk_t *chunk);



//  This func delete the value
//  - ARGUMENTS
//      sset - sorted set pointer
//      value - value to delete
//
//  - RETURNED VALUE
//      all is good => 0
//      set pointer is NULL => EINVAL
//      such value doesn't exist => ENOKEY
int ss_del(sset_t *sset, off_t index, off_t len);



//  This func finds the value and
//  gives ss_item_t pointer to the item parameter
//  - ARGUMENTS
//      sset - sorted set pointer
//      value - value to find
//      item - pointer to pointer where will be
//             written finded value
//
//  - RETURNED VALUE
//      all is good => 0
//      set pointer is NULL => EINVAL
//      such value doesn't exis => ENOKEY
int ss_find(sset_t *sset, off_t index, off_t len, chunk_t **chunk);



//  This func prints the set
//  - ARGUMENTS
//      sset - sorted set pointer
//
//  - RETURNED VALUE
//      all is good => 0
//      sset pointer is NULL => EINVAL
int ss_print(sset_t *sset, unsigned int from, unsigned int to);



#endif // SORTED_SET_H
