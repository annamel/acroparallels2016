#include "mapped_file.h"
#include "../mf/mf.h"
#include <errno.h>
#include <fcntl.h>

//TODO: Use max_memory_usage
mf_handle_t mf_open(const char *pathname, size_t max_memory_usage){
	return (mf_handle_t) _mf_open(pathname, O_RDWR | O_CREAT, 0755);
}

int mf_close(mf_handle_t mf){
	_mf_close((struct _mf *)mf);
	return 0;
}

ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void *buf){
	_mf_seek((struct _mf *)mf, offset);
	return _mf_read((struct _mf *)mf, buf, size);
}

ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void *buf){
	_mf_seek((struct _mf *)mf, offset);
	return _mf_write((struct _mf *)mf, buf, size);
}

mf_mapmem_t *mf_map(mf_handle_t mf, off_t offset, size_t size){
  	return _mf_map((struct _mf *)mf, offset, size);
}

int mf_unmap(mf_mapmem_t *mm){
	return _mf_unmap(mm);
}

ssize_t mf_file_size(mf_handle_t mf){
	return ((struct _mf *)mf) -> size;
}
