#include "mapped_file.h"
#include "../logger/log.h"
#include <stdlib.h>
#include <fcntl.h>

#define MIN(x,y) (x < y? x: y)

int main(int argc, char *argv[]){
	LOG(DEBUG, "---Open\n");
	struct mf *file1 = mf_open(argv[1], 12345);
	int file2 = open(argv[1], O_RDONLY);

	mf_mapmem_t * mm = mf_map(file1, 0, 12345);

	void *buf1 = malloc(12345);
	void *buf2 = malloc(12345);

	int len = MIN(mf_file_size(file1), 12345);

	memcpy(buf1, mm -> ptr, len);
	read(file2, buf2, 12345);

	mf_unmap(mm);

	if (memcmp(buf1, buf2, 12345))
		return 1;

 	mf_close(file1);
 	close(file2);
 	free(buf1);
 	free(buf2);
	return 0;
}
