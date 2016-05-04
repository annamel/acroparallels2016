#include "../../../include/mapped_file.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define GB 1024LL*1024LL*1024LL
#define FILENAME "test_file_dkopyrin"
#define FILESIZE 8*GB
#define MB 1024LL*1024LL

int main(){
	mf_handle_t file = mf_open(FILENAME);
	mf_mapmem_handle_t handle;

	void *ptr = mf_map(file, 0, FILESIZE, &handle);
	char out[4096];

	long it = 0;
	for (it = 0; it < FILESIZE; it += rand() % 4096){
		if (mf_read(file, out, 4096, it) == -1)
			return 1;
	}

	if (ptr) mf_unmap(file, handle);
	mf_close(file);
	return 0;
}
