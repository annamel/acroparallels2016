#include "../../../include/mapped_file.h"
/*
 * Test ladder
 * ----------
 * This test works nearly the same as test_ht but does not make first chunk.
 * That is why special chunk structure is generated:
 * CCCCCCCCC
 *    CCCCCCCCCC
 *        CCCCCCCC
 *           CCCCCCCCCC
 *            CCCCCCCCCCCC
 * If your algorithm has low granularity performance will be very bad because
 * for each request algorithm will have to make new map.
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#define GB 1024LL*1024LL*1024LL
#define FILENAME "test_file_dkopyrin"
#define MB 1024LL*1024LL

#define FILESIZE 8*GB
#define SAMPLESIZE 100*MB
#define VAC(command) do{								\
	struct rlimit rl_good = {.rlim_cur=128*GB, .rlim_max=-1};		\
	struct rlimit rl_bad = {.rlim_cur=128*MB, .rlim_max=-1};		\
	if (setrlimit(RLIMIT_AS, &rl_bad))						\
		printf("Fail_bad, %s\n", strerror(errno));			\
	command;									\
	if (setrlimit(RLIMIT_AS, &rl_good))					\
		printf("Fail_good, %s\n", strerror(errno));			\
}while(0)

int main(){
	mf_handle_t file;
	VAC(file = mf_open(FILENAME));

	long it = 0;
	long err_count = 0;
	for (it = 0; it < FILESIZE; it += rand() % 1024){
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
