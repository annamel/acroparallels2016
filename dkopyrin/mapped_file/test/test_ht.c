#include "../../../include/mapped_file.h"
/*
 * Test hashtable
 * --------------
 * This test maps part of file(1GB) and holds lock till the end of test.
 * In order to have good performance your program preferably should not make
 * any new maps but use the existing and increase reference counter of chunk.
 * Some performance problems might appear if you add to hashtable to many offsets
 * to search so your hashtable starts to work slow. Increasing granularity
 * or using hashtable not to hold all offsets might increase performance
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define GB 1024LL*1024LL*1024LL
#define FILENAME "test_file_dkopyrin"
#define MB 1024LL*1024LL

int main(){
	mf_handle_t file = mf_open(FILENAME);
	mf_mapmem_handle_t handle;

	mf_map(file, 0, GB, &handle);

	long it = 0;
	for (it = 0; it < GB; it += rand() % MB){
		mf_mapmem_handle_t loc_handle;
		void *loc_ptr = mf_map(file, it, MB, &loc_handle);
		if (loc_ptr == NULL){
                     if(errno != EINVAL)
			    return 1;
		}else{
		     mf_unmap(file, loc_handle);
              }
	}

	mf_unmap(file, handle);
	mf_close(file);
	return 0;
}
