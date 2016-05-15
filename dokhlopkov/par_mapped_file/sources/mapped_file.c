#define _XOPEN_SOURCE 500

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <semaphore.h>

#include "mapped_file.h"
#include "chunks.h"

mf_handle_t mf_open(const char* name) {
    if (name == NULL) return MF_OPEN_FAILED;
    mf_handle_t mf;

    int fd = open(name, O_RDWR | O_CREAT, 0666);
    if (fd == -1) return MF_OPEN_FAILED;

    cpool_t *cpool = NULL;
    int err = cpool_construct(0, fd, &cpool);
    if (err) return MF_OPEN_FAILED;

    mf = (void *)cpool;
    return mf;
}

int mf_close(mf_handle_t mf) {
    if (!mf) return EAGAIN;
    cpool_t *cpool = (cpool_t *)mf;
    sem_wait(&cpool->sem);
    int err = cpool_destruct((cpool_t *)mf);
    sem_post(&cpool->sem);
    return err ? -1 : 0;
}

ssize_t mf_read(mf_handle_t mf, void *buf, size_t size, off_t offset) {
    cpool_t *cpool = (cpool_t *)mf;
    chunk_t *ch = NULL;
    ssize_t read_bytes = 0;
    sem_wait(&cpool->sem);
    int err = ch_find(cpool, offset, size, &ch);
    void *src = NULL;
    switch(err) {
    case 0:
        err = ch_get_mem(ch, offset, &src);
        if (err) {
            sem_post(&cpool->sem);
            return -1;
        }
        memcpy(buf, src, size);
        read_bytes = size;
        break;
    case ENODATA:
        read_bytes = pread(cpool_fd(cpool), buf, size, offset);
        if (read_bytes == -1) {
            sem_post(&cpool->sem);
            return -1;
        }
        break;
    default:
        sem_post(&cpool->sem);
        return -1;
    }
    sem_post(&cpool->sem);
    return read_bytes;
}

ssize_t mf_write(mf_handle_t mf, const void *buf, size_t size, off_t offset) {
    cpool_t *cpool = (cpool_t *)mf;
    chunk_t *ch = NULL;
    ssize_t written_bytes = 0;

    sem_wait(&cpool->sem);
    errno = ch_find(cpool, offset, size, &ch);

    void *dst = NULL;
    switch(errno) {
    case 0:
        errno = ch_get_mem(ch, offset, &dst);
        if (errno) {
            sem_post(&cpool->sem);
            return -1;
        }
        memcpy(dst, buf, size);
    case ENODATA:
        written_bytes = pwrite(cpool_fd(cpool), buf, size, offset);
        if (written_bytes == -1) {
            sem_post(&cpool->sem);
            return -1;
        }
        break;
    default:
        sem_post(&cpool->sem);
        return -1;
    }
    sem_post(&cpool->sem);
    return written_bytes;
}

void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle) {
    if (size <= 0) return NULL;
    if (mf == MF_OPEN_FAILED || mapmem_handle == NULL || offset < 0 || mf_file_size(mf) < size + offset) {
        errno = EINVAL;
        return MF_MAP_FAILED;
    }

    cpool_t *cpool = (cpool_t *)mf;
    chunk_t **ch_ptr = (chunk_t **)mapmem_handle;

    sem_wait(&cpool->sem);
    errno = ch_acquire(cpool, offset, size, ch_ptr);
    if (errno) return MF_MAP_FAILED;

    void *ptr = NULL;
    errno = ch_get_mem(*ch_ptr, offset, &ptr);
    sem_post(&cpool->sem);
	  return errno ? MF_MAP_FAILED : ptr;
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle) {
  cpool_t *cpool = (cpool_t *)mf;
    if (mapmem_handle == NULL) {
        errno = EINVAL;
        return -1;
    }
  sem_wait(&cpool->sem);
	errno = ch_release((chunk_t *)mapmem_handle);
  sem_post(&cpool->sem);
	return errno ? -1: 0;
}

off_t mf_file_size(mf_handle_t mf) {
	off_t size = 0;
  int fd = cpool_fd((cpool_t *)mf);
	struct stat sb = {0};
	int err = fstat(fd, &sb);
	if (err == -1) return -1;
	size = sb.st_size;
	return err ? -1 : size;
}

