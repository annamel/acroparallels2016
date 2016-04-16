#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "mapped_file.h"
#include "chunks.h"
#include "mallocs.h"



mf_handle_t mf_open(const char* name, size_t max_memory) {
  if (name == NULL) return MF_OPEN_FAILED;
  mf_handle_t mf;

  int fd = open(name, O_RDWR, S_IWRITE | S_IREAD);
  if (fd == -1) return MF_OPEN_FAILED;

  cpool_t *cpool = NULL;
  int err = cpool_construct(max_memory, fd, &cpool);
  if (err) return MF_OPEN_FAILED;

  mf = (void *)cpool;
  return mf;
}

int mf_close(mf_handle_t mf) {
  if (!mf) return EAGAIN;
  return cpool_destruct((cpool_t **)&mf);
}

ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void *buf) {
  cpool_t *cpool = (cpool_t *)mf;
  chunk_t *chunk = NULL;
  ssize_t read_bytes = 0;
  int err = ch_find(cpool, offset, size, &chunk);

  void *src = NULL;
  switch(err) {
    case 0:
      err = ch_get_mem(chunk, offset, &src);
      if (err) return err;
      memcpy(buf, src, size);
      read_bytes = size;
      break;
    case ENODATA:
      read_bytes = pread(cpool_fd(cpool), buf, size, offset);
      if (read_bytes == -1) return errno;
      break;
    default:
      return err;
      break;
  }
  return read_bytes;
}

ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, void *buf) {
  cpool_t *cpool = (cpool_t *)mf;
  chunk_t *chunk = NULL;
  ssize_t written_bytes = 0;
  int err = ch_find(cpool, offset, size, &chunk);

  void *dst = NULL;
  switch(err) {
    case 0:
      err = ch_get_mem(chunk, offset, &dst);
      if (err) return err;
      memcpy(dst, buf, size);
      written_bytes = size;
    case ENODATA:
      written_bytes = pwrite(cpool_fd(cpool), buf, size, offset);
      if (written_bytes == -1) return errno;
      break;
    default:
      return err;
      break;
  }
  return 0;
}

mf_mapmem_t *mf_map(mf_handle_t mf, off_t offset, size_t size) {
  mf_mapmem_t * mapmem;
	errno = _malloc(sizeof(mf_mapmem_t), (void **)&mapmem);
	if(errno) return MF_MAP_FAILED;
	mapmem->handle = NULL;

	cpool_t *cpool = (cpool_t *)mf;
	chunk_t *chunk = (chunk_t *)mapmem->handle;

	errno = ch_acquire(cpool, offset, size, &chunk);
	if(errno) return MF_MAP_FAILED;

	errno = ch_get_mem(chunk, offset, &mapmem->ptr);
	return errno ? MF_MAP_FAILED : mapmem;
}

int mf_unmap(mf_mapmem_t *mm) {
	errno = ch_release((chunk_t *)mm->handle);
	if(errno) return -1;
	errno = _free(sizeof(mf_mapmem_t), (void **)&mm);
	return errno ? -1: 0;
}

ssize_t mf_file_size(mf_handle_t mf) {
	off_t size = 0;
  int fd = cpool_fd( (cpool_t *)mf );
	struct stat sb = {0};
	int err = fstat(fd, &sb);
	if(err == -1) return errno;
	size = sb.st_size;
	return errno ? (ssize_t)size : -1;
}
