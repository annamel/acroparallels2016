#include "chunk.h"

#define COLOR(x) "\x1B[33m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"
#include "chunk_manager.h"
#include <sys/mman.h>
#include <fcntl.h>

#define MAX(x,y) ((x > y) ? x: y)
#define MIN(x,y) ((x < y) ? x: y)

int chunk_manager_init (struct chunk_manager *cm, int fd, int mode){
	LOG(INFO, "chunk_manager_init called\n");
	cm -> fd = fd;
	hashtable_init(&cm -> ht, 65535);
	int i;
	for (i = 0; i < POOL_SIZE; i++){
		chunk_init_unused(cm -> chunk_pool + i);
	}

	int prot = 0;
	LOG(INFO, "mode: %d, %d %d %d\n", mode & 3, O_RDONLY, O_WRONLY, O_RDWR);
	if ((mode & 3) == O_RDONLY) prot = PROT_READ;
	//TODO: Not supported
	//if ((mode & 3) == O_WRONLY) prot = PROT_WRITE;
	if ((mode & 3) == O_RDWR) prot = PROT_READ | PROT_WRITE;
	LOG(INFO, "prot: %d, %d %d\n", prot, PROT_READ, PROT_WRITE);
	cm -> prot = prot;
	cm -> cur_chunk_index = 0;
	return 0;
}

int chunk_manager_finalize (struct chunk_manager *cm){
	LOG(INFO, "chunk_manager_finalize called\n");
	hashtable_finalize(&cm -> ht);
	int i;
 	for (i = 0; i < POOL_SIZE; i++)
 		if (cm -> chunk_pool[i].state != -1)
			chunk_finalize (cm -> chunk_pool + i);
	return 0;
}

struct chunk *chunk_manager_get_av_chunk_index (struct chunk_manager *cm){
	LOG(INFO, "chunk_manager_get_av_chunk_index called\n");
	int unav_chunk_cnt = 0;

	while (unav_chunk_cnt < POOL_SIZE) {
		struct chunk *cur_ch = cm -> chunk_pool + cm -> cur_chunk_index;

		cm -> cur_chunk_index = (cm -> cur_chunk_index + 1) % POOL_SIZE;
		if (cur_ch -> state == -1){
			LOG(DEBUG, "Unused chunk %d returned\n", (cm -> cur_chunk_index - 1) % POOL_SIZE);
			return cur_ch;
		}else{
			LOG(DEBUG, "Refirbished chunk %d returned\n", (cm -> cur_chunk_index - 1) % POOL_SIZE);
			if (cur_ch -> ref_cnt == 0){
				chunk_finalize(cur_ch);
				return cur_ch;
			}
		}
		unav_chunk_cnt++;
	}
	return NULL;
}

long int chunk_manager_offset2chunk (struct chunk_manager *cm, long int offset, long int length, struct chunk ** ret_ch, int *chunk_offset, int remap) {
	LOG(INFO, "offset2chunk called\n");
	long int poffset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
	long int plength = ((offset + length) & ~(sysconf(_SC_PAGE_SIZE) - 1)) +
		sysconf(_SC_PAGE_SIZE) - poffset;

	hashtable_value_t val = hashtable_get (&cm -> ht, poffset);
	struct chunk *cur_ch = (struct chunk *)val.ptr;

	if (cur_ch == NULL || cur_ch -> state != val.state || (remap && cur_ch -> length - offset + cur_ch -> offset < length)) {
		LOG(DEBUG, "No chunk found - making new one of size %d\n", plength);
		//TODO: Logics on generating new chunk
		struct chunk *new_chunk = chunk_manager_get_av_chunk_index(cm);
		if (new_chunk == NULL)
			return -1;

		chunk_init (new_chunk, MAX(MIN_CHUNK_SIZE, plength), poffset, cm -> prot, cm -> fd);

		long int new_chunk_offset = new_chunk -> offset;
		long int new_chunk_length = new_chunk -> length;
	  	long int hashtable_offset = 0;
		for(hashtable_offset = 0; hashtable_offset < new_chunk_length; hashtable_offset += sysconf(_SC_PAGE_SIZE), new_chunk_offset += sysconf(_SC_PAGE_SIZE)){
			LOG(DEBUG, "Adding offset %d to hashtable\n", new_chunk_offset);
			hashtable_value_t val = {new_chunk, new_chunk -> state};
			hashtable_set (&cm -> ht, new_chunk_offset, val);
		}
		*ret_ch = new_chunk;
		*chunk_offset = offset - new_chunk -> offset;
		return new_chunk_length - offset + new_chunk -> offset;
	}else{
		LOG(DEBUG, "Chunk found - nice!\n");
		*chunk_offset = offset - cur_ch -> offset;
		*ret_ch = cur_ch;
		//if (remap && cur_ch -> length - offset + cur_ch -> offset < length)
		//	chunk_remap(cur_ch, plength);
		return cur_ch -> length - offset + cur_ch -> offset;
	}
}
