#include "chunk.h"

#define COLOR(x) "\x1B[33m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"
#include "chunk_manager.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <limits.h>

#include "../nbds/include/skiplist.h"
#include "../nbds/include/runtime.h"

int skiplist_cmp(void *l, void *r){
	return (off_t) l - (off_t) r;
}

void *skiplist_clone(void * ptr){
       return ptr;
}

const datatype_t sdt = {.cmp = skiplist_cmp, .clone = skiplist_clone};

int chunk_manager_init (struct chunk_manager *cm, int fd, int mode){
	LOG(INFO, "chunk_manager_init called\n");
       assert(cm);
       cm -> fd = fd;
	int i;
	for (i = 0; i < POOL_SIZE; i++){
		chunk_init_unused(cm -> chunk_pool + i);
	}

	cm -> cur_chunk_index = 0;
       nbd_thread_init();
       cm -> skiplist = sl_alloc(&sdt);
	if (!cm -> skiplist)
		return 1;

	return 0;
}

int chunk_manager_finalize (struct chunk_manager *cm){
	LOG(INFO, "chunk_manager_finalize called\n");
	assert(cm);
	int i;
 	for (i = 0; i < POOL_SIZE; i++)
 		if (cm -> chunk_pool[i].ref_cnt != -1)
			chunk_finalize (cm -> chunk_pool + i);

       LOG(INFO, "sl fini\n");
	sl_free(cm -> skiplist);
#ifdef MEMORY_DEBUG
	cm -> fd = 0xDEAD;
	cm -> skiplist = (void *) 0xDEADBEEF;
#endif
  	return 0;
}

struct chunk *chunk_manager_get_av_chunk_from_pool (struct chunk_manager *cm){
	LOG(INFO, "chunk_manager_get_av_chunk_from_pool called\n");
	assert(cm);
	int end_index = (cm -> cur_chunk_index - 1) & (POOL_SIZE - 1);
       LOG(DEBUG, "End index is %d, start is %d\n", end_index, cm -> cur_chunk_index);
       //FIFO algorithm
	for (; cm -> cur_chunk_index != end_index; cm -> cur_chunk_index++){
		struct chunk *cur_ch = cm -> chunk_pool + cm -> cur_chunk_index;
		//By default ref_cnt == -1 is unused chunk
		LOG(DEBUG, "Trying %d chunk\n", cm -> cur_chunk_index);
		if (cur_ch -> ref_cnt == -1){
			LOG(DEBUG, "Unused chunk %d returned\n", cm -> cur_chunk_index);
                     cm -> cur_chunk_index++;
			return cur_ch;
		}else if (cur_ch -> ref_cnt == 0){
			LOG(DEBUG, "Refirbished chunk %d returned, cleaning\n", cm -> cur_chunk_index);
                     sl_remove(cm -> skiplist, cur_ch -> offset);

			chunk_finalize(cur_ch);
                     cm -> cur_chunk_index++;
			return cur_ch;
		}//else we can't use this chunk: ref_cnt != 0
	}
	return NULL;
}

long int chunk_manager_gen_chunk (struct chunk_manager *cm, off_t offset, size_t length, struct chunk ** ret_ch, off_t *chunk_offset) {
	LOG(INFO, "offset2chunk called\n");
	assert(cm);
	assert(ret_ch);
	assert(chunk_offset);
	//All chunks are aligned by CHUNK_MASK to improve search perfomance
	off_t poffset = offset & CHUNK_MASK;
	size_t plength = ((offset + length) & CHUNK_MASK) + MIN_CHUNK_SIZE - poffset;

       LOG(DEBUG, "Lookuping with %lld\n", poffset);
       struct chunk *cur_ch = (struct chunk *)sl_lookup(cm -> skiplist, poffset);

	/* This algorithm is optimistic: we believe, that chunk with nearest offset
	 * is the one that we want to find which is not true in every case
	 */
	//TODO: implement interval tree or structure that allows to find biggest chunk

	//off_t relative_offset = poffset - cur_ch -> offset;
	//ssize_t relative_length = plength - cur_ch -> offset;
       LOG(DEBUG, "Found nice chunk %p\n", cur_ch);
	if (cur_ch != NULL) LOG(DEBUG, "Closest chunk is offset %d, size %d\n", cur_ch -> offset, cur_ch -> length);
	if (cur_ch == NULL ||
	    cur_ch -> length < poffset - cur_ch -> offset ||
	    cur_ch -> length < plength) {
              LOG(DEBUG, "No chunk found - making new one of size %lld\n", plength);

		struct chunk *new_chunk = chunk_manager_get_av_chunk_from_pool(cm);
		if (new_chunk == NULL)
			return -1;

		chunk_init (new_chunk, plength, poffset, cm -> fd);

		LOG(DEBUG, "Adding offset %d to skiplist w/ chunk %p\n", new_chunk -> offset, new_chunk);
              //This function actually do add to skiplist
		map_val_t ret = sl_cas(cm -> skiplist, poffset, CAS_EXPECT_DOES_NOT_EXIST, (unsigned long) new_chunk);
              if (ret == DOES_NOT_EXIST) {
                     LOG(DEBUG, "Added new, %d\n", ret);
              }else{
                     LOG(DEBUG, "Replaced, %d\n", ret);
              }


		*ret_ch = new_chunk;
		*chunk_offset = offset - new_chunk -> offset;
		return new_chunk -> length - offset + new_chunk -> offset;
	}else{
		LOG(DEBUG, "Skiplist lookup success!\n");
		*chunk_offset = offset - cur_ch -> offset;
		*ret_ch = cur_ch;
		return cur_ch -> length - offset + cur_ch -> offset;
	}
}
