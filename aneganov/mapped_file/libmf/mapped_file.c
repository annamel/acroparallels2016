#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stddef.h>

#include "chunk_manager.h"
#include "mf_malloc.h"
#include "log.h"
#include "__mapped_file.h"
#include "mapped_file.h"

mf_handle_t mf_open(const char* pathname, size_t max_memory_usage) {
	mf_handle_t mf;
	errno = __mf_open(pathname, max_memory_usage, O_RDWR, 0666, &mf);
	return errno ? MF_OPEN_FAILED : mf;
}

int mf_close(mf_handle_t mf) {
	errno = __mf_close(mf);
	return errno ? -1 : 0;
}

ssize_t mf_read (mf_handle_t mf, off_t offset, size_t size, void* buf) {
	ssize_t read_bytes;
	errno = __mf_read(mf, offset, size, &read_bytes, buf);
	return read_bytes;
}

ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void* buf) {
	ssize_t written_bytes;
	errno = __mf_write(mf, offset, size, &written_bytes, buf);
	return written_bytes;	
}

mf_mapmem_t *mf_map(mf_handle_t mf, off_t offset, size_t size) {
	mf_mapmem_t * mapmem;
	errno = mf_malloc( sizeof(mf_mapmem_t), (void **)&mapmem );
	if(errno) return MF_MAP_FAILED;
	mapmem->handle = NULL;

	chpool_t *cpool = (chpool_t *)mf;
	chunk_t *chunk = (chunk_t *)mapmem->handle;

	errno = chunk_acquire(cpool, offset, size, &chunk);
	if(errno) return MF_MAP_FAILED;

	errno = chunk_get_mem(chunk, offset, &mapmem->ptr);
	return errno ? MF_MAP_FAILED : mapmem;
}

int mf_unmap(mf_mapmem_t *mm) {
	errno = chunk_release((chunk_t *)mm->handle);
	if(errno) return -1;
	errno = mf_free(sizeof(mf_mapmem_t), (void **)&mm);
	return errno ? -1: 0;
}

ssize_t mf_file_size(mf_handle_t mf) {
	off_t size = 0;
	errno = __mf_file_size(mf, &size);
	return errno ? (ssize_t)size : -1;
}
