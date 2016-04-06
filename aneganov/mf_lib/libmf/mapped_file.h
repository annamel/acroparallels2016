#ifndef __MAPPED_FILE__
#define __MAPPED_FILE__

#include <sys/types.h>
#include <errno.h>

typedef void* mf_handle_t;

int mf_open(const char* name, size_t max_memory, mf_handle_t* mf);
int mf_close(mf_handle_t mf);
int mf_map(mf_handle_t mf, off_t offset, size_t size, void** ptr);
int mf_unmap(mf_handle_t mf, size_t size, void** ptr);
int mf_file_flush(mf_handle_t* mf);
int mf_read(mf_handle_t mf, off_t offset, size_t size, void* buf);
int mf_write(mf_handle_t mf, off_t offset, size_t size, void* buf);

#endif
