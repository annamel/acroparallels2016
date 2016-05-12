#include "../../../include/mapped_file.h"
/*
 * Test OOM (Out-of-memory)
 * -----------------------
 * This test tries to read file that cannot be fully inserted into RAM.
 * That is why your algorithm will have to clean up previously mapped chunks
 * not to be killed by Out-of-memory killer.
 */

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
	for (it = 0; it < FILESIZE - 4096; it += 4096){
		int ret = mf_read(file, out, 4096, it);
		if (ret != 4096){
			printf("Was able to read only %d/4096 bytes from %ld/%ld\n", ret, it, FILESIZE);
			return 1;
		}
	}

	if (ptr) mf_unmap(file, handle);
	mf_close(file);
	return 0;
}
