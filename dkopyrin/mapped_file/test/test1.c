#include "mapped_file.h"
#include "../logger/log.h"
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define MIN(x,y) (x < y? x: y)

int main(int argc, char *argv[]){
	LOG(DEBUG, "---Open\n");
	struct mf *file1 = mf_open(argv[1]);
	int file2 = open(argv[1], O_RDONLY);

	mf_mapmem_handle_t mh;

	void *ptr = mf_map(file1, 0, 123, &mh);
	if (ptr == NULL){
		return mf_file_size(file1) > 123;
	}

	void *buf1 = malloc(123);
	void *buf2 = malloc(123);

	int len = MIN(mf_file_size(file1), 123);

	memcpy(buf1, ptr, len);
	read(file2, buf2, 123);

	mf_unmap(file1, mh);

	if (memcmp(buf1, buf2, 123))
		return 1;

 	mf_close(file1);
 	close(file2);
 	free(buf1);
 	free(buf2);
	return 0;
}
