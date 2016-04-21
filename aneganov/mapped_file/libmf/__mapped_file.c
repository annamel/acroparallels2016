#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include "chunk_manager.h"
#include "mfdef.h"
#include "mf_iterator.h"
#include "log.h"
#include "mapped_file.h"

int __mf_open(const char* name, size_t max_memory, int flags, int perms, mf_handle_t* mf) {
	if(name == NULL || mf == NULL)
		return EINVAL;

	//flags &= !O_CREAT;
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
	*read_bytes = 0;
	log_write(LOG_INFO, "__mf_read: start reading from file...\n");

	struct mf_iter it;
	int err = mf_iter_init(cpool, offset, size, &it);
	if(unlikely(err)) return err;

	while( !mf_iter_empty(&it) ) {
		if(it.ptr) {
			log_write(LOG_DEBUG, "__mf_read: copying from the chunk\n");
			memcpy(buf, it.ptr, it.step_size);
			*read_bytes += it.step_size;
		}
		else {
			log_write(LOG_DEBUG, "__mf_read: reading data explicitly\n");
			ssize_t pread_size = pread(chpool_fd(cpool), buf, it.step_size, it.offset);
			if(pread_size == -1) return errno;
			*read_bytes += pread_size;
			if(pread_size < it.step_size)
				break;
		}
		buf += it.step_size;
		err = mf_iter_next(&it);
		if(unlikely(err)) return err;
	}

	return 0;
}

int __mf_write(mf_handle_t mf, off_t offset, size_t size, ssize_t *written_bytes, const void* buf) {
	chpool_t *cpool = (chpool_t *)mf;
	*written_bytes = 0;
	log_write(LOG_INFO, "__mf_write: start writing to file...\n");


	struct mf_iter it;
	int err = mf_iter_init(cpool, offset, size, &it);
	if(unlikely(err)) return err;

	while( !mf_iter_empty(&it) ) {
		if(it.ptr) {
			log_write(LOG_DEBUG, "__mf_write: copying to the chunk\n");
			memcpy(it.ptr, buf, it.step_size);
			*written_bytes += it.step_size;
		}
		else {
			log_write(LOG_DEBUG, "__mf_write: writing data explicitly\n");
			ssize_t pwrite_size = pwrite(chpool_fd(cpool), buf, it.step_size, it.offset);
			if(pwrite_size == -1) return errno;
			*written_bytes += pwrite_size;
			if(pwrite_size < it.step_size)
				break;
		}
		buf += it.step_size;
		err = mf_iter_next(&it);
		if(unlikely(err)) return err;
	}

	return 0;
}

int __mf_acquire(mf_handle_t mf, off_t offset, size_t size, void** ptr) {
	chpool_t *cpool = (chpool_t *)mf;
	chunk_t *chunk = NULL;

	int err = chunk_acquire(cpool, offset, size, &chunk);
	if(err) return err;

	err = chunk_get_mem(chunk, offset, ptr);
	if(unlikely(err)) return err;

	err = chpool_mem_add(*ptr, chunk);
	return err;
}

int __mf_release(mf_handle_t mf, size_t dummy, void* ptr) {
	chpool_t *cpool = (chpool_t *)mf;
	chunk_t *chunk = NULL;

	int err = chpool_mem_get(cpool, ptr, &chunk);
	if(unlikely(err)) return err;

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
