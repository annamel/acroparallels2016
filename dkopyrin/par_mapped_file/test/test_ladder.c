#include "../../../include/mapped_file.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define GB 1024LL*1024LL*1024LL
#define FILENAME "test_file_dkopyrin"
#define MB 1024LL*1024LL

#define FILESIZE 8*GB
#define SAMPLESIZE 100*MB

int main(){
	mf_handle_t file = mf_open(FILENAME);

	long it = 0;
	long err_count = 0;
	for (it = 0; it < FILESIZE; it += rand() % MB){
		mf_mapmem_handle_t loc_handle;
		void *loc_ptr = mf_map(file, it, MB, &loc_handle);
		if (loc_ptr == NULL){
			err_count++;
			if (errno != EINVAL)
				return 1;
		}else{
		     mf_unmap(file, loc_handle);
              }
	}
	printf("Errors: %ld\n", err_count);

	char *buf = malloc(SAMPLESIZE);
	if (buf == NULL)
		return 2;
	long ret = mf_read(file, buf, SAMPLESIZE, FILESIZE - SAMPLESIZE - 1);
	if (ret != SAMPLESIZE)
		return 3;
	free(buf);

	mf_close(file);
	return 0;
}
