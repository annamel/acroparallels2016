#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H



#include "sys/mman.h"



#include "dc_list/dc_list.h"
#include "hash_table/hash_table.h"



#define DEFAULT_HASHTABLE_SIZE 16
#define DEFAULT_ARRAY_SIZE 16



#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a > _b ? _a : _b; })



typedef struct chunk chunk_t;
typedef struct chpool chpool_t;



struct chunk
{
    off_t index;
    off_t len;
    unsigned int rc;
    void *data;
    chpool_t *chpool;
};



//  The pool of chunks structure
//  All chunks lie in hash table lists by indexes,
//  which determine by offset
//  All free chunks(rc = 0) lie in free_ch list
//
//      fd - descriptor of file
//      prot - desired memory protection
//      ht - hash table of chunks
//      zero_list - list of chunks with zero ref counter
//      free_list - list of free chunks
//      pool - array of chunk arrays
//             each of arrays has
struct chpool
{
    int fd;
    int prot;
    size_t arrays_cnt;
    chunk_t **pool;
    htable_t *ht;
    dclist_t *zero_list;
    dclist_t *free_list;
};




chunk_t *ch_init(off_t index, off_t len, chpool_t *chpool);

size_t get_chunk_size(off_t multiplier);


chpool_t *chp_init(int fd, int prot);



//  This func returns chunk which contains finded chunk
int chp_find(chpool_t *chpool, off_t index, off_t len, chunk_t **chunk);


int chp_chunk_release(chunk_t *chunk);

#endif // CHUNK_MANAGER_H
