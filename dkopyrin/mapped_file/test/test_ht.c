#include "../../../include/mapped_file.h"

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
