#include "chunk.h"

#define COLOR(x) "\x1B[33m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"
#include "chunk_manager.h"
#ifdef DANGER_MMAP
#define __USE_GNU
#endif
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

struct chunk search_chunk = {0, NULL, 0, 0, 0};

int chunk_cmp(const void *l, const void *r){
	return (off_t)(((struct chunk *)l) -> offset)
	     - (off_t)(((struct chunk *)r) -> offset);
}

void chunk_free_node(void *ptr){
	((struct chunk *) ptr) -> rbnode = NULL;
}

int chunk_manager_init (struct chunk_manager *cm, int fd, int mode){
	LOG(INFO, "chunk_manager_init called\n");
	assert(cm);
	cm -> cur_chunk_size = DEFAULT_CHUNK_SIZE;
	cm -> fd = fd;
	int i;
	for (i = 0; i < POOL_SIZE; i++){
		chunk_init_unused(cm -> chunk_pool + i);
	}

	cm -> cur_chunk_index = 0;
	cm -> rbtree = rbtree_create(chunk_cmp, chunk_free_node, 1);
	if (!cm -> rbtree)
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

	rbtree_free(cm -> rbtree);
#ifdef MEMORY_DEBUG
	cm -> fd = 0xDEAD;
	cm -> rbtree = (void *) 0xDEADBEEF;
#endif
  	return 0;
}

struct chunk *chunk_manager_get_av_chunk_from_pool (struct chunk_manager *cm){
	LOG(INFO, "chunk_manager_get_av_chunk_from_pool called\n");
	assert(cm);
	unsigned end_index = (cm -> cur_chunk_index - 1) & (POOL_SIZE - 1);
       LOG(INFO, "end is %d, cur is %d\n", end_index, cm -> cur_chunk_index);
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
			if (cur_ch -> rbnode){
				LOG(DEBUG, "Removing from rbtree\n");
				rbtree_delete(cm -> rbtree, cur_ch -> rbnode);
			}
			LOG(DEBUG, "Current size of rbtree is %d\n", rbtree_num_elements(cm -> rbtree));

			chunk_finalize(cur_ch);
                     cm -> cur_chunk_index++;
			return cur_ch;
		}//else we can't use this chunk: ref_cnt != 0
	}
	return NULL;
}

ssize_t chunk_manager_gen_chunk (struct chunk_manager *cm, off_t offset, size_t size, struct chunk ** ret_ch, off_t *chunk_offset) {
	LOG(INFO, "chunk_manager_gen_chunk called\n");
	assert(cm);
	assert(ret_ch);
	assert(chunk_offset);
	//All chunks are aligned by cur_chunk_mask to improve search perfomance
	//cur_chunk_size might change it's size during execution to fit changing
	//conditions like failed to map so big chunk size
	off_t poffset = offset & MASK(cm -> cur_chunk_size);
	size_t psize = ((offset + size) & MASK(cm -> cur_chunk_size)) + cm -> cur_chunk_size - poffset;

	search_chunk.offset = poffset;
	struct chunk *cur_ch = (struct chunk *)rbtree_finddata(cm -> rbtree, &search_chunk);

	/* This algorithm is optimistic: we believe, that chunk with nearest offset
	 * is the one that we want to find which is not true in every case
	 */
	//TODO: implement interval tree or structure that allows to find biggest chunk

	//off_t relative_offset = offset - cur_ch -> offset;
	//ssize_t relative_size = size;
	//LOG(DEBUG, "Found nice chunk %p\n", cur_ch);
	if (cur_ch != NULL) LOG(DEBUG, "Closest chunk is offset %lld, size %lld\n", cur_ch -> offset, cur_ch -> size);
	if (cur_ch == NULL ||
	    offset + size > cur_ch -> size + cur_ch -> offset ) {
		LOG(DEBUG, "No chunk found - making new one of size %ld\n", psize);
		struct chunk *new_chunk = chunk_manager_get_av_chunk_from_pool(cm);
		if (new_chunk == NULL)
			return -1;

		while (1){
			LOG(DEBUG, "Trying size %ld\n", cm -> cur_chunk_size);
			poffset = offset & MASK(cm -> cur_chunk_size);
			psize = ((offset + size) & MASK(cm -> cur_chunk_size)) + cm -> cur_chunk_size - poffset;

			if (!chunk_init (new_chunk, psize, poffset, cm -> fd))
				break;
			cm -> cur_chunk_size >>= 1;
			if (cm -> cur_chunk_size <= MIN_CHUNK_SIZE)
				return -1;
		}
		if (cm -> cur_chunk_size < MAX_CHUNK_SIZE){
			cm -> cur_chunk_size <<= 1;
		}
	  	/* We can make adding to rbtree O(1) if we use offset that we already
		 * found to make less tree traversals
		 */
		//LOG(DEBUG, "Adding offset %lld to rbtree\n", new_chunk -> offset);
		if (cur_ch && cur_ch -> offset == new_chunk -> offset && cur_ch -> rbnode){
			cur_ch -> rbnode -> Data = new_chunk;
			cur_ch -> rbnode = NULL;
		}else{
			new_chunk -> rbnode = rbtree_insert(cm -> rbtree, new_chunk);

		}
		*ret_ch = new_chunk;
		*chunk_offset = offset - new_chunk -> offset;
		return new_chunk -> size - offset + new_chunk -> offset;
	}else{
		//LOG(DEBUG, "Rbtree lookup success!, %ld %ld %ld\n", cur_ch -> size, poffset - cur_ch -> offset, psize);
		*chunk_offset = offset - cur_ch -> offset;
		*ret_ch = cur_ch;
		return cur_ch -> size - offset + cur_ch -> offset;
	}
}
