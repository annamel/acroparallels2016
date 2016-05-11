#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H



#include "sys/mman.h"
#include "unistd.h"



#include "../dc_list/dc_list.h"
#include "../sorted_set/sorted_set.h"
#include "../typedefs.h"



#define DEFAULT_ARRAY_SIZE 1024



#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a > _b ? _a : _b; })



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
//      sset - sorted set of chunks for finding
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
    sset_t *sset;
    dclist_t *zero_list;
    dclist_t *free_list;
};



//  Initialize new chunk
//  - ARGUMENTS
//      index - offset in pagesizes
//      len - size in pagesizes
//      chpool - pool of chunks which contains this chunk
//  - RETURNED VALUE
//
chunk_t *ch_init(off_t index, off_t len, chpool_t *chpool);



//  Deinitialize a chunk
//  IT DOESN'T FREE CHUNK POINTER!
//  - ARGUMENTS
//      chunk - chunk to deinit
int ch_deinit(chunk_t *chunk);



//  Release a chunk
int ch_release(chunk_t *chunk);



//  This func returns a size of chunk
//  by its lenght in pagesizes
size_t get_chunk_size(off_t multiplier);



//*****************************************************************************
//*****************                                        ********************
//*****************            Chunk pool funcs            ********************
//*****************                                        ********************
//*****************************************************************************
//  Initialize new chunk pool attached to file
//  - ARGUMENTS
//      fd - file descriptor
//      prot - memory protection
chpool_t *chp_init(int fd, int prot);



//  Deinitialize chunk pool
//  IT DOESN'T FREE CHUNK POOL POINTER!
int chp_deinit(chpool_t *chpool);



//  This func returns chunk which contains finded chunk
int chp_find(chpool_t *chpool, off_t index, off_t len, chunk_t **chunk);



#endif // CHUNK_MANAGER_H
