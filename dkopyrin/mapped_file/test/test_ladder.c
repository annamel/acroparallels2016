#include "../../../include/mapped_file.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define GB 1024LL*1024LL*1024LL
#define FILENAME "test_size_file"
#define MB 1024LL*1024LL

int main(){
	int fd = open(FILENAME, O_RDWR | O_CREAT, 0777);
	ftruncate(fd, 8*GB);
	close(fd);

	mf_handle_t file = mf_open(FILENAME);
	mf_mapmem_handle_t handle;

	long it = 0;
	for (it = 0; it < 8*GB; it += rand() % MB){
		mf_mapmem_handle_t loc_handle;
		void *loc_ptr = mf_map(file, it, MB, &loc_handle);
		if (loc_ptr == NULL && errno != EINVAL){
			return 1;
		}
		mf_unmap(file, loc_handle);
	}

	mf_close(file);

	remove(FILENAME);
	return 0;
}
