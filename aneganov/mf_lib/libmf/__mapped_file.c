#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include "chunk_manager.h"
#include "log.h"
#include "mapped_file.h"

int __mf_open(const char* name, size_t max_memory, int flags, int perms, mf_handle_t* mf) {
	if(name == NULL || mf == NULL)
		return EINVAL;

	flags |= O_CREAT;
	int fd = open(name, flags, perms);
	if(fd == -1) return errno;

	chpool_t *cpool = NULL;

	int prot = (flags & O_RDWR) ? PROT_READ | PROT_WRITE : (flags & O_RDONLY) ? PROT_READ : PROT_WRITE;
	int err = chpool_construct(max_memory, fd, prot, &cpool);
	if(err) return err;

	*mf = (void *)cpool;
	return 0;
}

int __mf_close(mf_handle_t mf) {
	if(!mf) return EAGAIN;
	return chpool_destruct((chpool_t **)&mf); 
}

int __mf_read(mf_handle_t mf, off_t offset, size_t size, ssize_t *read_bytes, void* buf) {
	chpool_t *cpool = (chpool_t *)mf;
	chunk_t *chunk = NULL;
	*read_bytes = 0;

	int err = chunk_find(cpool, offset, size, &chunk);

	void *src = NULL;
	switch(err) {
		case 0:
			err = chunk_get_mem(chunk, offset, &src);
			if(err) return err;
			memcpy(buf, src, size);
			*read_bytes = size;
			break;
		case ENOKEY:
			*read_bytes = pread(chpool_fd(cpool), buf, size, offset);
			if(*read_bytes == -1) return errno;
			break;
		default:
			return err;
			break;
	}
	return 0;
}

int __mf_write(mf_handle_t mf, off_t offset, size_t size, ssize_t *written_bytes, const void* buf) {
	chpool_t *cpool = (chpool_t *)mf;
	chunk_t *chunk = NULL;
	*written_bytes = 0;

	int err = chunk_find(cpool, offset, size, &chunk);

	void *dst = NULL;
	switch(err) {
		case 0:
			err = chunk_get_mem(chunk, offset, &dst);
			if(err) return err;
			memcpy(dst, buf, size);
			*written_bytes = size;
			break;
		case ENOKEY:
			*written_bytes = pwrite(chpool_fd(cpool), buf, size, offset);
			if(*written_bytes == -1) return errno;
			break;
		default:
			return err;
			break;
	}
	return 0;	
}

int __mf_acquire(mf_handle_t mf, off_t offset, size_t size, void** ptr) {
	chpool_t *cpool = (chpool_t *)mf;
	chunk_t *chunk = NULL;

	int err = chunk_acquire(cpool, offset, size, &chunk);
	if(err) return err;

	err = chunk_get_mem(chunk, offset, ptr);
	if(err) return err;

	err = chpool_mem_add(ptr, chunk);
	return err;
}

int __mf_release(mf_handle_t mf, size_t dummy, void* ptr) {
	chpool_t *cpool = (chpool_t *)mf;
	chunk_t *chunk = NULL;

	int err = chpool_mem_get(cpool, ptr, &chunk);
	if(err) return err;

	err = chunk_release(chunk);
	return err;
}

int __mf_file_size(mf_handle_t mf, off_t *size) {
	int fd = chpool_fd( (chpool_t *)mf );
	struct stat sb = {0};
	int err = fstat(fd, &sb);
	if(err == -1) return errno;
	*size = sb.st_size;
	return 0;
}
