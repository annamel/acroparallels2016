
#include "mapped_file_final_api.h"

#ifdef __cplusplus
extern "C"
{
#endif 

#define MF_MAP_FAILED NULL

mf_handle_t mf_open(const char* pathname, size_t max_memory_usage){
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

ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void* buf){
	mf_mapmem_t* mapped_mem = mf_map(mf, offset, size);
	
	memcpy(buf, mapped_mem->ptr, size);
	mf_unmap(mapped_mem);
	return (ssize_t) size;
}

ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void* buf){
	mf_mapmem_t* mapped_mem = mf_map(mf, offset, size);
	
	memcpy(mapped_mem->ptr, buf, size);
	mf_unmap(mapped_mem);
	return (ssize_t) size;
}

mf_mapmem_t* mf_map(mf_handle_t mf, off_t offset, size_t size){
	mf_mapmem_t* mapped_mem = (mf_mapmem_t*) malloc(sizeof(mf_mapmem_t));
	my_mf_handle_t* my_mf = (my_mf_handle_t*) mf;
	if (!mapped_mem){
		errno = ENOMEM;
		return NULL;
	}
	mapped_mem->handle = malloc(sizeof(my_mf_mapmem_handle_t));
	if (!mapped_mem->handle){
		errno = ENOMEM;
		return NULL;
	}
	my_mf_mapmem_handle_t* map_mem_handle = (my_mf_mapmem_handle_t*) mapped_mem->handle;
	map_mem_handle -> offset = (offset/sysconf(_SC_PAGE_SIZE))*sysconf(_SC_PAGE_SIZE);
	map_mem_handle -> size = size+offset%sysconf(_SC_PAGE_SIZE);
	mapped_mem->ptr = mmap(NULL, size+offset%sysconf(_SC_PAGE_SIZE), 
		PROT_READ|PROT_WRITE, MAP_SHARED, my_mf->fd, 
		(offset/sysconf(_SC_PAGE_SIZE))*sysconf(_SC_PAGE_SIZE));
	if (mapped_mem->ptr == MAP_FAILED){
		free(map_mem_handle);
		free(my_mf);
		return NULL;
	}
	mapped_mem->ptr = (char*)mapped_mem->ptr + offset%sysconf(_SC_PAGE_SIZE);
	return mapped_mem;
}

int mf_unmap(mf_mapmem_t* mm){
	if (!mm)
		return -1;
	my_mf_mapmem_handle_t* map_mem_handle = (my_mf_mapmem_handle_t*) mm->handle;
	if (!map_mem_handle)
		return -1;
	munmap(mm->ptr, map_mem_handle->size);
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


