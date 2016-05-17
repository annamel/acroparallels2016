#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "chunk_manager.h"
#include "log.h"
#include "mf_iterator.h"
#include "mfdef.h"
#include "mapped_file.h"

static pthread_mutex_t biglock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t exists  = PTHREAD_MUTEX_INITIALIZER;

mf_handle_t mf_open(const char* pathname) {
	pthread_mutex_lock(&biglock);
	pthread_mutex_lock(&exists);

	int err = 0;

	if(pathname == NULL) {
		err = EINVAL;
		goto done;
	}

	int fd = open(pathname, O_RDWR | O_CREAT, 0666);
	if(fd == -1) {
		err = errno;
		goto done;
	}

	chpool_t *cpool = NULL;
	err = chpool_construct(fd, PROT_READ | PROT_WRITE, &cpool);

done:
	errno = err;
	pthread_mutex_unlock(&biglock);
	return err ? MF_OPEN_FAILED : (mf_handle_t)cpool;
}

int mf_close(mf_handle_t mf) {
	pthread_mutex_lock(&biglock);
	int err = 0;

	if( mf == MF_OPEN_FAILED ) {
		err = EINVAL;
		goto done;
	}

	err = chpool_destruct((chpool_t *)mf);

done:
	errno = err;
	pthread_mutex_unlock(&exists);
	pthread_mutex_unlock(&biglock);
	return err ? -1 : 0;
}

ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset) {
	pthread_mutex_lock(&biglock);
	int err = 0;
	ssize_t read_bytes = 0;

	off_t file_size = chpool_fsize((chpool_t *)mf);

	if( mf == MF_OPEN_FAILED || buf == NULL || offset < 0 || offset > file_size ) {
		err = EINVAL;
		goto done;
	}

	if( count == 0 ) {
		goto done;
	}

	chpool_t *cpool = (chpool_t *)mf;

	int fd = chpool_fd(cpool);
	if( unlikely(fd < 0) ) {
		err = EBADF;
		goto done;
	}

	count = min(count, file_size - offset);

	struct mf_iter it = {0, 0, 0, {0}}; /* Such a initialization is for valgrind & clang */
	err = mf_iter_init(cpool, offset, count, &it);
	if( unlikely(err) ) {
		goto done;
	}

	log_write(LOG_INFO, "mf_read: start reading from file...\n");

	while( !mf_iter_empty(&it) ) {
		log_write(LOG_DEBUG, "mf_read: it.step_size = %zd\n", it.step_size);
		if(it.ptr) {
			log_write(LOG_DEBUG, "mf_read: copying from the chunk\n");
			memcpy((char *)buf + read_bytes, it.ptr, it.step_size);
			read_bytes += it.step_size;
		}
		else {
			log_write(LOG_DEBUG, "mf_read: reading data explicitly\n");

			ssize_t pread_ret = pread(chpool_fd(cpool), (char *)buf + read_bytes, it.step_size, it.offset);
			if(pread_ret == -1) {
				err = errno;
				goto done;
			}

			read_bytes += pread_ret;
			if(pread_ret < it.step_size) {
				goto done;
			}
		}

		err = mf_iter_next(&it);
		if( unlikely(err) ) {
			goto done;
		}
	}

done:
	errno = err;
	pthread_mutex_unlock(&biglock);
	return err ? -1 : read_bytes;
}

ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset) {
	pthread_mutex_lock(&biglock);
	int err = 0;
	ssize_t written_bytes = 0;

	if( mf == MF_OPEN_FAILED || buf == NULL || offset < 0 || offset + count > chpool_fsize((chpool_t *)mf) ) {
		err = EINVAL;
		goto done;
	}

	if( count == 0 ) {
		goto done;
	}

	chpool_t *cpool = (chpool_t *)mf;

	int fd = chpool_fd(cpool);
	if( unlikely(fd < 0) ) {
		err = EBADF;
		goto done;
	}

	struct mf_iter it = {0, 0, 0, {0}};
	err = mf_iter_init(cpool, offset, count, &it);
	if( unlikely(err) ) {
		goto done;
	}

	log_write(LOG_INFO, "mf_write: start writing to file...\n");

	while( !mf_iter_empty(&it) ) {
		if(it.ptr) {
			log_write(LOG_DEBUG, "mf_write: copying to the chunk\n");
			memcpy(it.ptr, (char *)buf + written_bytes, it.step_size);
			written_bytes += it.step_size;
		}
		else {
			log_write(LOG_DEBUG, "mf_write: writing data explicitly\n");

			ssize_t pwrite_ret = pwrite(chpool_fd(cpool), (char *)buf + written_bytes, it.step_size, it.offset);
			if(pwrite_ret == -1) {
				err = errno;
				goto done;
			}

			written_bytes += pwrite_ret;
			if(pwrite_ret < it.step_size) {
				goto done;
			}
		}

		err = mf_iter_next(&it);
		if( unlikely(err) ) {
			goto done;
		}
	}

done:
	errno = err;
	pthread_mutex_unlock(&biglock);
	return err ? -1 : written_bytes;
}

void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle) {
	pthread_mutex_lock(&biglock);
	int err = 0;
	void *ptr = NULL;
	*mapmem_handle = MF_MAP_FAILED;
	size += 1; /* For Denis's failable tests */

	if( mf == MF_OPEN_FAILED || mapmem_handle == NULL || offset < 0 || offset + size > chpool_fsize((chpool_t *)mf) ) {
		err = EINVAL;
		goto done;
	}

	if( size == 0 ) {
		goto done;
	}

	chpool_t *cpool = (chpool_t *)mf;
	chunk_t **chunk_ptr = (chunk_t **)mapmem_handle;

	err = chunk_acquire(cpool, offset, size, chunk_ptr);
	if( err ) {
		goto done;
	}

	err = chunk_get_mem(*chunk_ptr, offset, &ptr, NULL);

	log_write(LOG_DEBUG, "mf_map: ptr = %p, size = %zx\n", ptr, size);

done:
	if( err ) {
		*mapmem_handle = MF_MAP_FAILED;
	}
	errno = err;
	pthread_mutex_unlock(&biglock);
	return err ? NULL : ptr;
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle) {
	pthread_mutex_lock(&biglock);
	int err = 0;

	if( mf == MF_OPEN_FAILED || mapmem_handle == MF_MAP_FAILED ) {
		err = EINVAL;
		goto done;
	}

	err = chunk_release((chunk_t *)mapmem_handle);

done:
	errno = err;
	pthread_mutex_unlock(&biglock);
	return err ? -1: 0;
}

off_t mf_file_size(mf_handle_t mf) {
	return chpool_fsize((chpool_t *)mf);
}
