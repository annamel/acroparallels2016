#include "mapped_file.h"
#include "../logger/log.h"
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]){
	if (argc < 2) {
		printf("Usage: %s filename\n", argv[0]);
		return 0;
	}

	LOG(DEBUG, "---Open\n");
	struct mf *file1 = mf_open(argv[1], 100500);
	int file2 = open(argv[1], O_RDONLY);

	void *buf1 = malloc(12345);
	void *buf2 = malloc(12345);

	LOG(DEBUG, "---Read 1\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
	mf_read(file1, 0, 5000, buf1);
	   read(file2, buf2, 5000);
	if (memcmp(buf1, buf2, 5000))
		return 1;
	LOG(DEBUG, "---Read 2\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
	mf_read(file1, 5000, 5000, buf1);
	   read(file2, buf2, 5000);
	//printf("%s", buf1);
  	//printf("\n\n\n\n\n\n\n\n\n");
  	//printf("%s", buf2);

	if(memcmp(buf1, buf2, 5000))
		return 2;
  	LOG(DEBUG, "---Read 3\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
	mf_read(file1, 10000, 2500, buf1);
	   read(file2, buf2, 2500);
	if (memcmp(buf1, buf2, 2500))
		return 3;
  	LOG(DEBUG, "---Read 4\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
  	  lseek(file2, 0, SEEK_SET);
	mf_read(file1, 0, 12345, buf1);
	   read(file2, buf2, 12345);
	if(memcmp(buf1, buf2, 12345))
		return 4;
	LOG(DEBUG, "---Close\n");

	mf_close(file1);
	close(file2);
	free(buf1);
	free(buf2);



	if (argc < 3) {
		printf("Write isage: %s filename_in filename_out\n", argv[0]);
		return 0;
	}
	LOG(DEBUG, "---Write\n");
	struct mf *f1 = mf_open(argv[1], 100500);
	struct mf *f2 = mf_open(argv[2], 100500);

  	void *buf = malloc(12345);

	LOG(DEBUG, "---Write 1\n");
	memset(buf, 0, 12345);
	int rb = mf_read (f1, 0, 12345, buf);
	mf_write(f2, 0, rb, buf);
	mf_close(f1);
	mf_close(f2);
	free(buf);
}
