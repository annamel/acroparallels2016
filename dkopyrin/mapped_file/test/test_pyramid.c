#include "../../../include/mapped_file.h"
/*
 * Test Pyramid
 * This test generates pyramidic chunk structure
 *                   CCCCCC
 *               CCCCCCCCCCCC
 *           CCCCCCCCCCCCCCCCCCCCC       |
 *        CCCCCCCCCCCCCCCCCCCCCCCCCCCC   V
 *    CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
 * CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
 * Such structure might generate a lot of failed lookups if algrorithm
 * has low granularity. After generating such structure read from end of file
 * is made. This way test checks whether algrorithm find right chunk to use
 * from pyramid, the biggest one.
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define GB 1024LL*1024LL*1024LL
#define FILENAME "test_file_dkopyrin"
#define MB 1024LL*1024LL

#define FILESIZE 4*GB
#define SAMPLESIZE 100*MB
#define CYCLES 1

#define HALFSIZE 2*GB

int main(){
	mf_handle_t file = mf_open(FILENAME);

	long it = 0;
	long last_ok = 0;
	for (it = 0; it < HALFSIZE; it += rand() % 512){
		mf_mapmem_handle_t loc_handle;
		void *loc_ptr = mf_map(file, HALFSIZE - it, 2 * it, &loc_handle);
		if (loc_ptr != NULL){
			last_ok = it;
			mf_unmap(file, loc_handle);
		}
	}

	printf("Last OK: %lgGB\n", ((double)last_ok)/((double)GB));

	char *buf = malloc(SAMPLESIZE);
  	if (buf == NULL)
		return 2;
	int i;
	for (i = 0; i < CYCLES; i++){
		long ret = mf_read(file, buf, SAMPLESIZE, last_ok - SAMPLESIZE);
		if (ret != SAMPLESIZE)
			return 3;
	}
	mf_close(file);
	return 0;
}
