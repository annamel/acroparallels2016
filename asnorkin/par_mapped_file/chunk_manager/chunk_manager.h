#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H



#include "sys/mman.h"
#include "unistd.h"
#include <pthread.h>



#include "../dc_list/dc_list.h"
#include "../hash_table/hash_table.h"
#include "../typedefs.h"



#define DEFAULT_ARRAY_SIZE 1024
#define DEFAULT_HT_SIZE 1024



#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a < _b ? _a : _b; })



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
//      arrays_cnt - number of loafs
//      sset - sorted set of chunks for finding
//      zero_list - list of chunks with zero ref counter
//      free_list - list of free chunks
//      pool - array of chunk arrays
//             each of arrays has
//      file_size - save filesize for perfomance
//      page_size - save pagesize for perfomance
struct chpool
{
    int fd;
    int prot;
    pthread_mutex_t flock;

    off_t file_size;
    off_t page_size;

    chunk_t *last_chunk;
    chunk_t *all_file;
    int is_all_map;

    size_t arrays_cnt;
    chunk_t **pool;

    sset_t *ht;
    pthread_mutex_t ht_write_lock;
    int ht_reads_numb;

    dclist_t *zero_list;
    pthread_mutex_t zl_write_lock;
    int zl_reads_numb;

    dclist_t *free_list;
    pthread_mutex_t fl_write_lock;
    int fl_reads_numb;
};



//  Initialize new chunk
//  - ARGUMENTS
//      index - offset in pagesizes
//      len - size in pagesizes
//      chpool - pool of chunks which contains this chunk
//
//  - RETURNED VALUE
//      all is good => new chunk pointer
//      something goes wrong => NULL(see logfile and errno)
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



//  This func returns filesize by file descriptor
off_t chp_file_size(int fd);



#endif // CHUNK_MANAGER_H
