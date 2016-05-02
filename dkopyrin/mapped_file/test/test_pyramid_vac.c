#include "../../../include/mapped_file.h"

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

#define FILESIZE 4*GB
#define SAMPLESIZE 100*MB
#define CYCLES 1

#define HALFSIZE 2*GB

//Virtual memory Anti-Cheat
//Limits virtual memory for 'command'
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
	long last_ok = 0;
	for (it = 0; it < HALFSIZE; it += rand() % 512){
		mf_mapmem_handle_t loc_handle;
		void *loc_ptr = mf_map(file, HALFSIZE - it, 2 * it, &loc_handle);
		if (loc_ptr != NULL){
			last_ok = it;
			mf_unmap(file, loc_handle);
		}
	}

	printf("Last OK: %lgGB\n", ((double)last_ok)/((double)GB));

	char *buf = malloc(SAMPLESIZE);
  	if (buf == NULL)
		return 2;
	int i;
	for (i = 0; i < CYCLES; i++){
		long ret = mf_read(file, buf, SAMPLESIZE, last_ok - SAMPLESIZE);
		if (ret != SAMPLESIZE)
			return 3;
	}
	mf_close(file);
	return 0;
}
