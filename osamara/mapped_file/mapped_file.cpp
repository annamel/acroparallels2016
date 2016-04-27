
#include "mapped_file.h"
#include "my_mapped_file.h"
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
	my_mf_handle_t * file_handler = (my_mf_handle_t*) malloc(sizeof(my_mf_handle_t));
	if (!file_handler){
		errno = ENOMEM;
		return NULL;
	}
	if ((file_handler->fd = open(pathname, O_RDWR, 0)) == -1){
		free (file_handler);
		return NULL;
	}
	return file_handler;
}
	
int mf_close(mf_handle_t mf){
	my_mf_handle_t * my_mf = (my_mf_handle_t *) mf;
	close (my_mf->fd);
	return 0;
}

ssize_t mf_read(mf_handle_t mf, void* buf, size_t size, off_t offset){
	mf_mapmem_handle_t* mapped_mem;
	void* buff = mf_map(mf, offset, size, mapped_mem);
	memcpy(buf, buff, size);
	mf_unmap(mf, mapped_mem);
	return (ssize_t) size;
}

ssize_t mf_write(mf_handle_t mf, const void* buf, size_t size, off_t offset){
	mf_mapmem_handle_t* mapped_mem;
	void* buff = mf_map(mf, offset, size, mapped_mem);
	memcpy(buff, buf, size);
	mf_unmap(mf, mapped_mem);
	return (ssize_t) size;
}

void* mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t* mapmem_handle){
	
	mapmem_handle = (mf_mapmem_handle_t*) malloc(sizeof(my_mf_mapmem_handle_t));
	
	if (!mapmem_handle){
		errno = ENOMEM;
		return NULL;
	}
	my_mf_handle_t* my_mf = (my_mf_handle_t*) mf;
	my_mf_mapmem_handle_t * my_mapmem_handle = (my_mf_mapmem_handle_t *) mapmem_handle;
	my_mapmem_handle -> offset = (offset/sysconf(_SC_PAGE_SIZE))*sysconf(_SC_PAGE_SIZE);
	my_mapmem_handle -> size = size+offset%sysconf(_SC_PAGE_SIZE);
	my_mapmem_handle->ptr = mmap(NULL, max(size+offset%sysconf(_SC_PAGE_SIZE),sysconf(_SC_PAGE_SIZE)), 
		PROT_READ|PROT_WRITE, MAP_SHARED, my_mf->fd, 
		(offset/sysconf(_SC_PAGE_SIZE))*sysconf(_SC_PAGE_SIZE));
	if (my_mapmem_handle->ptr == MAP_FAILED){
		free(my_mapmem_handle);
		return NULL;
	}
	my_mapmem_handle->ptr = (char*)my_mapmem_handle->ptr + offset%sysconf(_SC_PAGE_SIZE);
	return (void*)my_mapmem_handle->ptr;
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle){
	if (!mapmem_handle)
		return -1;
	my_mf_mapmem_handle_t* map_mem_handle = (my_mf_mapmem_handle_t*) mapmem_handle;
	if (!map_mem_handle)
		return -1;
	munmap(map_mem_handle->ptr, map_mem_handle->size);
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


