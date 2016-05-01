#include "../../../include/mapped_file.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#define GB 1024LL*1024LL*1024LL
#define FILENAME "test_file_dkopyrin"
#define FILESIZE 8*GB
#define MB 1024LL*1024LL

#define SAMPLESIZE 100*MB

void *test_ht(void *file){
	long it = 0;
	long err_count = 0;
	for (it = 0; it < FILESIZE; it += rand() % MB){
		mf_mapmem_handle_t loc_handle;
		void *loc_ptr = mf_map(file, it, MB, &loc_handle);
		if (loc_ptr == NULL){
			err_count++;
			if (errno != EINVAL)
				return (void *)1;
		}else{
		     mf_unmap(file, loc_handle);
              }
	}
	printf("Errors: %ld\n", err_count);

	char *buf = malloc(SAMPLESIZE);
	if (buf == NULL)
		return (void *)2;
	long ret = mf_read(file, buf, SAMPLESIZE, FILESIZE - SAMPLESIZE - 1);
	if (ret != SAMPLESIZE)
		return (void *)3;
	free(buf);

	return 0;
}

int main(int argc, char *argv[]){
       if (argc < 2){
              printf("Usage: %s num_threads\n", argv[0]);
              return -1;
       }
       int num_threads = atoi(argv[1]);
       if (num_threads <= 0){
              printf("Bad num of threads\n");
              return -2;
       }

       mf_handle_t file = mf_open(FILENAME);

       pthread_t *threads = (pthread_t *) malloc(num_threads * sizeof(pthread_t));
       void ** rets = malloc(num_threads * sizeof(void *));

       int i;
       for (i = 0; i < num_threads; i++){
              if(pthread_create(threads+i, NULL, test_ht, file)){
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

	free(rets); free(threads);
       mf_close(file);
	return 0;
}
