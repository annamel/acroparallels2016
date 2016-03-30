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
	struct mf * file1 = malloc(sizeof(struct mf));
	mf_open(argv[1], file1);
	int file2 = open(argv[1], O_RDONLY);

  	void *buf1 = calloc(12345, 1);
  	void *buf2 = calloc(12345, 1);

	LOG(DEBUG, "---Read 1\n");
	mf_read(file1, buf1, 5000);
	   read(file2, buf2, 5000);
	printf("Test1: %d\n", memcmp(buf1, buf2, 5000));
	LOG(DEBUG, "---Read 2\n");
	mf_read(file1, buf1, 5000);
	   read(file2, buf2, 5000);
	printf("Test2: %d\n", memcmp(buf1, buf2, 5000));
  	LOG(DEBUG, "---Read 3\n");
	mf_read(file1, buf1, 2500);
	   read(file2, buf2, 2500);
	printf("Test3: %d\n", memcmp(buf1, buf2, 2500));
  	LOG(DEBUG, "---Read 4\n");
  	mf_seek(file1, 0);
  	  lseek(file2, 0, SEEK_SET);
	mf_read(file1, buf1, 12345);
	   read(file2, buf2, 12345);
	printf("Test4: %d\n", memcmp(buf1, buf2, 12345));
	LOG(DEBUG, "---Close\n");

	mf_close(file1);
	free(file1);
	close(file2);
	free(buf1);
	free(buf2);
}
