#include "../../../include/mapped_file.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#define GB 1024LL*1024LL*1024LL
#define FILENAME "test_file_dkopyrin"
#define MB 1024LL*1024LL

void *test_ht(void *file){
       long it = 0;
	for (it = 0; it < GB; it += 666){
		mf_mapmem_handle_t loc_handle;
		void *loc_ptr = mf_map(file, it, MB, &loc_handle);
		if (loc_ptr == NULL && errno != EINVAL){
			return (void *)1;
		}
              if (loc_ptr)
		     mf_unmap(file, loc_handle);
	}

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
	mf_mapmem_handle_t handle;
       mf_map(file, 0, GB, &handle);

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

       //mf_unmap(file, handle);
       mf_close(file);
       free(rets); free(threads);
	return 0;
}
