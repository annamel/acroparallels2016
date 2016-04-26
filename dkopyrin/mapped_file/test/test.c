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
	struct mf *file1 = mf_open(argv[1]);
	int file2 = open(argv[1], O_RDONLY);

	void *buf1 = malloc(12345);
	void *buf2 = malloc(12345);

	LOG(DEBUG, "---Read 1\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
	mf_read(file1, buf1, 5000, 0);
	   read(file2, buf2, 5000);
	if (memcmp(buf1, buf2, 5000))
		return 1;
	LOG(DEBUG, "---Read 2\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
	mf_read(file1, buf1, 5000, 5000);
	   read(file2, buf2, 5000);
	//printf("%s", buf1);
  	//printf("\n\n\n\n\n\n\n\n\n");
  	//printf("%s", buf2);

	if(memcmp(buf1, buf2, 5000))
		return 2;
  	LOG(DEBUG, "---Read 3\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
	mf_read(file1, buf1, 2500, 10000);
	   read(file2, buf2, 2500);
	if (memcmp(buf1, buf2, 2500))
		return 3;
  	LOG(DEBUG, "---Read 4\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
  	  lseek(file2, 0, SEEK_SET);
	mf_read(file1, buf1, 12345, 0);
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
	struct mf *f1 = mf_open(argv[1]);
	struct mf *f2 = mf_open(argv[2]);

  	void *buf = malloc(20000);

	LOG(DEBUG, "---Write 1\n");
	memset(buf, 0, 20000);
	int rb = mf_read (f1, buf, 20000, 0);
	mf_write(f2, buf, rb, 0);
	mf_close(f1);
	mf_close(f2);
	free(buf);
}
