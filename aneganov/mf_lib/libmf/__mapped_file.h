#ifndef __MAPPED_FILE_DEP__
#define __MAPPED_FILE_DEP__

#include <sys/types.h>
#include <errno.h>

typedef void* mf_handle_t;

int __mf_open(const char* name, size_t max_memory, int flags, int perms, mf_handle_t* mf);
int __mf_close(mf_handle_t mf);
int __mf_acquire(mf_handle_t mf, off_t offset, size_t size, void** ptr);
int __mf_release(mf_handle_t mf, size_t size, void* ptr);
int __mf_read(mf_handle_t mf, off_t offset, size_t size, ssize_t *read_bytes, void* buf);
int __mf_write(mf_handle_t mf, off_t offset, size_t size, ssize_t *written_bytes, const void* buf);
int __mf_file_size(mf_handle_t mf, off_t *size);

#endif
