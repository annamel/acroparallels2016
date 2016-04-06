#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stddef.h>

#include "chunk_manager.h"
#include "mapped_file.h"

int mf_open(const char* name, size_t max_memory, mf_handle_t* mf) {
	if(name == NULL || mf == NULL)
		return EINVAL;

	int fd = open(name, O_RDWR, 0666);
	if(fd == -1) return errno;

	chpool_t *cpool = NULL;
	int err = chpool_construct(max_memory, fd, PROT_READ | PROT_WRITE, &cpool);
	if(err) return err;

	*mf = (void *)cpool;
	return 0;
}

int mf_close(mf_handle_t mf) {
	if(!mf) return EAGAIN;
	return chpool_destruct((chpool_t **)&mf); 
}

int mf_map(mf_handle_t mf, off_t offset, size_t size, void** ptr) {
	chpool_t *cpool = (chpool_t *)mf;
	chunk_t *chunk = NULL;

	int err = chunk_acquire(cpool, offset, size, &chunk);
	if(err) return err;

	err = chunk_get_mem(chunk, offset, ptr);
	if(err) return err;

	return 0;
}

#if 0
int mf_unmap(mf_handle_t mf, size_t size, void** ptr) {
	chpool_t *cpool = (chpool_t *)mf;
	chunk_t *chunk = NULL;
	//TODO: add search for chunk by ptr and write this function
	return 0;
}

int mf_read(mf_handle_t mf, off_t offset, size_t size, void* buf) {
	chpool_t *cpool = (chpool_t *)mf;
	chunk_t *chunk = NULL;

	int err = chunk_find(cpool, offset, size, &chunk);

	switch(err) {
		case 0:
			break;
		case ENOKEY:
			break;
		default:
			return err;
			break;
	}

	return 0;
}

int mf_write(mf_handle_t mf, off_t offset, size_t size, void* buf) {
	return 0;
}

#endif