
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
	 handle_t * mf = ( handle_t*) malloc(sizeof( handle_t));
	if (!mf){
		//printf("Alloc fail\n");
		errno = ENOMEM;
		return NULL;
	}
	if ((mf->fd = open(pathname, O_RDWR, 0)) == -1){
		//printf("Open fail\n");
		free (mf);
		return NULL;
	}
	mf->mapped = 0;
	mf->r_map = ( mapmem_handle_t *) malloc(sizeof( mapmem_handle_t));
	mf->r_map->offset = 0;
	mf->r_map->size = 0;
	mf->r_map->ptr = NULL;
	mf->r_map->true_ptr = NULL;
	
	struct stat st;
	fstat(mf->fd, &st);
	mf->r_map->true_ptr = mmap(NULL, st.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, mf->fd, 0);
	if (mf->r_map->true_ptr != MAP_FAILED){
		mf->r_map->size = st.st_size;
		mf->r_map->ptr = mf->r_map->true_ptr;
		mf->mapped = 1;
	}
	//file->file_size = st.st_size;
	//file->page_size = (size_t) sysconf(_SC_PAGE_SIZE);
	
	//printf("open took %d\n",time_ms()-time_start);
	return (mf_handle_t)mf;
}
	
int mf_close(mf_handle_t mf){
	 handle_t * my_mf = ( handle_t *) mf;
	close (my_mf->fd);
	free(my_mf->r_map);
	free(my_mf);
	return 0;
}
// mapmem_handle_t *  mf->r_map = NULL;
ssize_t mf_read(mf_handle_t mf, void* buf, size_t size, off_t offset){
	mf_mapmem_handle_t mapped_mem = NULL;

	void* buff = mf_map(mf, offset, size, &mapped_mem);

	memcpy(buf, buff, size);
	mf_unmap(mf, mapped_mem);
	return (ssize_t) size;
}

ssize_t mf_write(mf_handle_t mf, const void* buf, size_t size, off_t offset){
	mf_mapmem_handle_t mapped_mem;
	void* buff = mf_map(mf, offset, size, &mapped_mem);
	memcpy(buff, buf, size);
	//mf_unmap(mf, mf_mapmem_hamdle_t);
	return (ssize_t) size;
}

void* mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t* mapmem_handle){
	
	handle_t* my_mf = (handle_t*) mf;
	//printf("Mappin %d %d; mapped %d %d\n", (int)offset, (int)size, (int)my_mf->r_map->offset, (int)my_mf->r_map->size);
	*mapmem_handle = (void*)  my_mf->r_map;
	if (my_mf->mapped){
		my_mf->r_map->ptr = (void*)((char*)my_mf->r_map->true_ptr + offset);
		return (void*) ( my_mf->r_map->ptr);
	}
	long long int pg_size=sysconf(_SC_PAGE_SIZE);
	
	long long int moffset = (offset/pg_size)*pg_size;
	long long int msize = min(max(size+offset%pg_size,pg_size*128*1024), mf_file_size(mf));
	if (my_mf->r_map->offset <= moffset && my_mf->r_map->size + my_mf->r_map->offset >= offset + size){
		//printf("yay\n");
		my_mf->r_map->ptr = (void*)((char*) my_mf->r_map->ptr + (moffset - my_mf->r_map->offset));
		return (void*) ( my_mf->r_map->ptr);
	}
	if(my_mf->r_map->offset != moffset || my_mf->r_map->size != msize){
		//HERE
		//printf("miss\n");
		munmap((void*)my_mf->r_map->true_ptr, (size_t)my_mf->r_map->size);
		my_mf->r_map->true_ptr = mmap(NULL, msize, PROT_READ|PROT_WRITE, 
			MAP_SHARED, my_mf->fd, moffset);
		if ( my_mf->r_map->true_ptr == MAP_FAILED){
			return NULL;
		}
		 my_mf->r_map->offset =  moffset;
		 my_mf->r_map->size =  msize;
		 my_mf->r_map->ptr = (char*) my_mf->r_map->true_ptr + offset%pg_size;
		*mapmem_handle =  my_mf->r_map;
	}
	return (void*) ( my_mf->r_map->ptr);
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle){
	if (!mapmem_handle)
		return -1;
	 mapmem_handle_t* map_mem_handle = ( mapmem_handle_t*) mapmem_handle;
	if (!map_mem_handle)
		return -1;
	//munmap((void*)map_mem_handle->true_ptr, (size_t)map_mem_handle->size);
	//if(map_mem_handle) {
	//	free(map_mem_handle);
	//}
	return 0;
}

ssize_t mf_file_size(mf_handle_t mf){
	struct stat buf;
	 handle_t * my_mf = ( handle_t *) mf;
	fstat(my_mf->fd, &buf);
	return buf.st_size;
}
#ifdef __cplusplus
}
#endif // __cplusplus


	
