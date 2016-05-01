
#include "mapped_file.h"
#include "my_mapped_file.h"
#include <stdio.h>
#include <sys/times.h>
#ifdef __cplusplus
extern "C"
{
#endif 

#define MF_MAP_FAILED NULL
uint64_t min (uint64_t a, uint64_t b){
	return a>b?b:a;
}
uint64_t max (uint64_t a, uint64_t b){
	return a<b?b:a;
}
mf_handle_t mf_open(const char* pathname){
	//long long time_start = time_ms();
	my_mf_handle_t * file_handler = (my_mf_handle_t*) malloc(sizeof(my_mf_handle_t));
	if (!file_handler){
		errno = ENOMEM;
		return NULL;
	}
	if ((file_handler->fd = open(pathname, O_RDWR, 0)) == -1){
		free (file_handler);
		return NULL;
	}
	//printf("open took %d\n",time_ms()-time_start);
	return file_handler;
}
	
int mf_close(mf_handle_t mf){
	my_mf_handle_t * my_mf = (my_mf_handle_t *) mf;
	close (my_mf->fd);
	free(my_mf);
	return 0;
}
my_mf_mapmem_handle_t * recent_map = NULL;
ssize_t mf_read(mf_handle_t mf, void* buf, size_t size, off_t offset){
	mf_mapmem_handle_t mapped_mem = NULL;

	void* buff = mf_map(mf, offset, size, &mapped_mem);

	memcpy(buf, buff, size);
	return (ssize_t) size;
}

ssize_t mf_write(mf_handle_t mf, const void* buf, size_t size, off_t offset){
	mf_mapmem_handle_t mapped_mem;
	void* buff = mf_map(mf, offset, size, &mapped_mem);
	memcpy(buff, buf, size);
	return (ssize_t) size;
}

void* mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t* mapmem_handle){
	
	if(!recent_map){
		recent_map = (my_mf_mapmem_handle_t *) malloc(sizeof(my_mf_mapmem_handle_t));
		recent_map->offset = 0;
		recent_map->size = 0;
		recent_map->ptr = NULL;
		recent_map->true_ptr = NULL;
	}
	*mapmem_handle = (void*) recent_map;
	int pg_size=sysconf(_SC_PAGE_SIZE);
	my_mf_handle_t* my_mf = (my_mf_handle_t*) mf;
	int moffset = (offset/pg_size)*pg_size;
	int msize = min(max(size+offset%pg_size,pg_size), mf_file_size(mf));
	if(recent_map->offset != moffset || recent_map->size != msize){
		recent_map->true_ptr = mmap(NULL, msize, PROT_READ|PROT_WRITE, 
			MAP_SHARED, my_mf->fd, moffset);
		if (recent_map->true_ptr == MAP_FAILED){
			return NULL;
		}
		recent_map->offset =  moffset;
		recent_map->size =  msize;
		recent_map->ptr = (char*)recent_map->true_ptr + offset%pg_size;
		*mapmem_handle = recent_map;
	}
	return (void*) (recent_map->ptr);
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle){
	if (!mapmem_handle)
		return -1;
	my_mf_mapmem_handle_t* map_mem_handle = (my_mf_mapmem_handle_t*) mapmem_handle;
	if (!map_mem_handle)
		return -1;
	munmap((void*)map_mem_handle->true_ptr, (size_t)map_mem_handle->size);
	if(map_mem_handle) {
		free(map_mem_handle);
	}
	return 0;
}

ssize_t mf_file_size(mf_handle_t mf){
	struct stat buf;
	my_mf_handle_t * my_mf = (my_mf_handle_t *) mf;
	fstat(my_mf->fd, &buf);
	return buf.st_size;
}
#ifdef __cplusplus
}
#endif // __cplusplus


	
