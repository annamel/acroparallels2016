#include "mf.h"
#include "../logger/log.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]){
	if (argc < 2) {
		printf("Usage: %s filename\n", argv[0]);
		return 0;
	}
	LOG(DEBUG, "---Open\n");
	struct mf *file1 = mf_open(argv[1], O_RDONLY, file1);
	int file2 = open(argv[1], O_RDONLY);

  	void *buf1 = malloc(12345);
  	void *buf2 = malloc(12345);

	LOG(DEBUG, "---Read 1\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
	mf_read(file1, buf1, 5000);
	   read(file2, buf2, 5000);
	printf("Test1: %d\n", memcmp(buf1, buf2, 5000));
	LOG(DEBUG, "---Read 2\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
	mf_read(file1, buf1, 5000);
	   read(file2, buf2, 5000);
	printf("Test2: %d\n", memcmp(buf1, buf2, 5000));
  	LOG(DEBUG, "---Read 3\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
	mf_read(file1, buf1, 2500);
	   read(file2, buf2, 2500);
	printf("Test3: %d\n", memcmp(buf1, buf2, 2500));
  	LOG(DEBUG, "---Read 4\n");
	memset(buf1, 0, 12345);
	memset(buf2, 0, 12345);
  	mf_seek(file1, 0);
  	  lseek(file2, 0, SEEK_SET);
	mf_read(file1, buf1, 12345);
	   read(file2, buf2, 12345);
	printf("Test4: %d\n", memcmp(buf1, buf2, 12345));
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
	struct mf *f1 = mf_open(argv[1], O_RDONLY);
	struct mf *f2 = mf_open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0777);

  	void *buf = malloc(12345);

	LOG(DEBUG, "---Write 1\n");
	memset(buf, 0, 12345);
	int rb = mf_read (f1, buf, 12345);
	mf_write(f2, buf, rb);
	mf_close(f1);
	mf_close(f2);
	free(buf);
}
