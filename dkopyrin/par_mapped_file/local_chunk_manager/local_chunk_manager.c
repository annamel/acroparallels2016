#include "local_chunk_manager.h"

#define COLOR(x) "\e[94m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"

#include "../chunk_manager/chunk_manager.h"
#include "../chunk_manager/chunk.h"

#include <string.h>

int local_chunk_manager_init (struct local_chunk_manager *lcm, int fd, int mode){
       int thr;
       int index;
       for (thr = 0; thr < MAX_NUM_THREADS; thr++){
              for (index = 0; index < LOCAL_POOL_SIZE; index++){
                     lcm -> lchunk_pool[thr][index].chunk = NULL;
                     lcm -> lchunk_pool[thr][index].local_ref_cnt = 0;
              }
              lcm -> cur_lchunk_index_free[thr] = 0;
              lcm -> cur_lchunk_index_prev[thr] = 0;
       }

       return chunk_manager_init(&lcm -> cm, fd, mode);
}
int local_chunk_manager_finalize (struct local_chunk_manager *lcm){
       return chunk_manager_finalize(&lcm -> cm);
}

ssize_t local_chunk_manager_gen_chunk (struct local_chunk_manager *lcm, off_t offset, size_t length, struct local_chunk ** ret_ch, off_t *chunk_offset){
       //LOG(INFO, "local_chunk_manager_gen_chunk called\n");
       struct local_chunk *thread_chunk_pool = lcm -> lchunk_pool[GET_THREAD_INDEX()];


       int cur_lchunk_index;
       int cache_miss = 0;
       int old_lchunk_index = lcm -> cur_lchunk_index_prev[GET_THREAD_INDEX()];
       int end_lchunk_index = (old_lchunk_index - 1 + LOCAL_POOL_SIZE) % LOCAL_POOL_SIZE;


       for (cur_lchunk_index = old_lchunk_index; cur_lchunk_index != end_lchunk_index; cur_lchunk_index=(cur_lchunk_index+1) % LOCAL_POOL_SIZE){
              struct local_chunk *local_cur_ch = &thread_chunk_pool[cur_lchunk_index];
              struct chunk *cur_ch = local_cur_ch -> chunk;

              size_t ch_length = cur_ch ? cur_ch -> length : -1;
              off_t ch_offset = cur_ch ? cur_ch -> offset: -1;
              //Search for good chunk - simple algo for now
              if (cur_ch && offset >= ch_offset && offset + length <= ch_length + ch_offset){
                     //LOG(INFO, "Lookup succeed\n");
                     if (cache_miss) {LOG(INFO, "Cache miss, %d\n", cache_miss);}
                     *ret_ch = local_cur_ch;
                     *chunk_offset = offset - ch_offset;
                     if (cache_miss) lcm -> cur_lchunk_index_prev[GET_THREAD_INDEX()] = cur_lchunk_index;
                     return ch_length - offset + ch_offset;
              }
              LOG(DEBUG, "Called with size %ld, offset %ld\n", length, offset);
              LOG(DEBUG, "Missed chunk with offset %ld, size %ld\n", ch_offset, ch_length);
              cache_miss++;
       }

       //LOG(INFO, "Lookup failed\n");
       //Chunk was not found - we need to generate a new one
       old_lchunk_index = lcm -> cur_lchunk_index_free[GET_THREAD_INDEX()];
       end_lchunk_index = (old_lchunk_index - 1  + LOCAL_POOL_SIZE) % LOCAL_POOL_SIZE;
       for (cur_lchunk_index = old_lchunk_index; cur_lchunk_index != end_lchunk_index; cur_lchunk_index=(cur_lchunk_index+1) % LOCAL_POOL_SIZE){
              struct local_chunk *local_cur_ch = &thread_chunk_pool[cur_lchunk_index];

              LOG(DEBUG, "Trying to free chunk %d with ref_cnt %d\n", cur_lchunk_index, local_cur_ch -> local_ref_cnt);
              if (local_cur_ch -> local_ref_cnt == 0){
                     struct chunk* cur_ch = local_cur_ch -> chunk;
                     lcm -> cur_lchunk_index_free[GET_THREAD_INDEX()] = (cur_lchunk_index + 1) % LOCAL_POOL_SIZE;

                     if (cur_ch) __atomic_fetch_sub(&cur_ch -> trc.ref_cnt, 1, 0);
                     struct chunk * new_chunk = NULL;
                     off_t ch_offset = 0;
                     size_t ch_length = 0;
                     ch_length = chunk_manager_gen_chunk(&lcm -> cm, offset, length, &new_chunk, &ch_offset);
                     if (ch_length == -1)
                            return -1;

                     LOG(DEBUG, "Got new chunk with size %ld, offset %ld\n", ch_length, ch_offset);

                     lcm -> cur_lchunk_index_prev[GET_THREAD_INDEX()] = cur_lchunk_index;

                     local_cur_ch -> chunk = new_chunk;
                     *ret_ch = local_cur_ch;
                     *chunk_offset = offset - new_chunk -> offset;

                     return new_chunk -> length - offset + new_chunk -> offset;
              }
       }

       return -1;
}
