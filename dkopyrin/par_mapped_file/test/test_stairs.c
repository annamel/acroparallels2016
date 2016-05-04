#include "../../../include/mapped_file.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

#define GB 1024LL*1024LL*1024LL
#define FILENAME "test_file_dkopyrin"
#define FILESIZE 6*GB
#define MB 1024LL*1024LL

#define SAMPLESIZE 128*MB

mf_handle_t file;
long int num_threads;
char * buf;

void *test_ladder(void *num){
	long int thread_num = (long int) num;
	long it = 0;
	long err_count = 0;

	long unit_map = FILESIZE / num_threads;
	long start = FILESIZE - unit_map * thread_num - unit_map;
	long end = FILESIZE - unit_map * thread_num;

	printf("%ld: %ld %ld\n", thread_num, start, end);

	for (it = start; it < end; it += 222){
		mf_mapmem_handle_t loc_handle;
		void *loc_ptr = mf_map(file, 0, it, &loc_handle);
		if (loc_ptr == NULL){
			err_count++;
			printf("%s\n", strerror(errno));
			if (errno != EINVAL)
				return (void *)1;
		}else{
			mf_unmap(file, loc_handle);
              }
	}
	printf("Errors: %ld:%ld\n",thread_num, err_count);

	long unit = SAMPLESIZE / num_threads;
	long buf_offset = unit * thread_num;

	long ret = mf_read(file, buf+buf_offset, unit, FILESIZE-SAMPLESIZE+buf_offset);
	if (ret != unit)
		return (void *)3;
	return 0;
}

int main(int argc, char *argv[]){
       if (argc < 2){
              printf("Usage: %s num_threads\n", argv[0]);
              return -1;
       }
       num_threads = atoi(argv[1]);
       if (num_threads <= 0){
              printf("Bad num of threads\n");
              return -2;
       }

       file = mf_open(FILENAME);

       pthread_t *threads = (pthread_t *) malloc(num_threads * sizeof(pthread_t));
       void ** rets = malloc(num_threads * sizeof(void *));
	buf = malloc(SAMPLESIZE);
	if (buf == NULL)
		return 2;

       int i;
       for (i = 0; i < num_threads; i++){
              if(pthread_create(threads+i, NULL, test_ladder, (void *)(long int)i)){
                     printf("Thread make fail");
                     exit(-3);
              }
       }
       for (i = 0; i < num_threads; i++){
              pthread_join(threads[i], rets + i);
       }
       for (i = 0; i < num_threads; i++){
              if (rets[i] != 0)
                     return (int)i;
       }

	free(rets); free(threads); free(buf);
       mf_close(file);
	return 0;
}
