#ifndef __LOCAL_CHUNK_MANAGER_H
#define __LOCAL_CHUNK_MANAGER_H

#include "../chunk_manager/chunk_manager.h"
#include "../nbds/runtime/rlocal.h"
#define LOCAL_POOL_SIZE 32

//Local chunk are used to make less ref_cnt atomic operations is workflow
//That's thread as whole blocks one chunk in global chunk_manager
//Local chunk manager doesn't have to use locks as they are considered to be
//non-parallel
struct local_chunk{
       struct chunk * chunk;
       int local_ref_cnt;
};

struct local_chunk_manager{
       struct chunk_manager cm;
       int cur_lchunk_index_prev[MAX_NUM_THREADS];
       int cur_lchunk_index_free[MAX_NUM_THREADS];
       struct local_chunk lchunk_pool[MAX_NUM_THREADS][LOCAL_POOL_SIZE];
};

int local_chunk_manager_init (struct local_chunk_manager *cm, int fd, int mode);
int local_chunk_manager_finalize (struct local_chunk_manager *cm);

ssize_t local_chunk_manager_gen_chunk (struct local_chunk_manager *cm, off_t offset, size_t length, struct local_chunk ** ret_ch, off_t *chunk_offset);

#endif
