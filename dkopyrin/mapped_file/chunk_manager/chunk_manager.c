#include "chunk.h"

#define COLOR(x) "\x1B[33m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"
#include "chunk_manager.h"
#include <sys/mman.h>

int chunk_manager_init (struct chunk_manager *cm, int fd){
	LOG(INFO, "chunk_manager_init called\n");
	cm -> fd = fd;
	hashtable_init(&cm -> ht, 65535);
	int i;
	for (i = 0; i < POOL_SIZE; i++){
		chunk_init_unused(cm -> chunk_pool + i);
	}
	return 0;
}

int chunk_manager_finalize (struct chunk_manager *cm){
	LOG(INFO, "chunk_manager_finalize called\n");
	hashtable_finalize(&cm -> ht);
	int i;
 	for (i = 0; i < POOL_SIZE; i++){
		chunk_finalize (cm -> chunk_pool + i);
	}
	return 0;
}

long int chunk_manager_offset2chunk (struct chunk_manager *cm, long int offset, long int length, struct chunk ** ret_ch, int *chunk_offset) {
	LOG(INFO, "offset2chunk called\n");
	long int poffset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
	long int plength = ((offset + length) & ~(sysconf(_SC_PAGE_SIZE) - 1)) +
		sysconf(_SC_PAGE_SIZE) - poffset;

	struct chunk *cur_ch = (struct chunk *) hashtable_get (&cm -> ht, poffset);
	if (cur_ch == NULL) {
		LOG(DEBUG, "No chunk found - making new one\n");
		//TODO: Logics on generating new chunk
		struct chunk *new_chunk = cm -> chunk_pool;
		if (new_chunk -> state != -1)
			chunk_finalize (new_chunk);
		chunk_init (new_chunk, plength, poffset, PROT_READ, cm -> fd);

		long int new_chunk_offset = 0;
		long int new_chunk_length = new_chunk -> length;
		for(new_chunk_offset = new_chunk -> offset; new_chunk_offset < new_chunk_length; new_chunk_offset += sysconf(_SC_PAGE_SIZE)){
			LOG(DEBUG, "Adding offset %d to hashtable\n", new_chunk_offset);
			hashtable_set (&cm -> ht, new_chunk_offset, new_chunk);
		}
		*ret_ch = cm -> chunk_pool;
		*chunk_offset = offset - poffset;
		return plength - offset + poffset;
	}else{
		LOG(DEBUG, "Chunk found - nice!\n");
		*chunk_offset = offset - poffset;
		*ret_ch = cur_ch;
		return cur_ch -> length - offset + poffset;
	}
}
