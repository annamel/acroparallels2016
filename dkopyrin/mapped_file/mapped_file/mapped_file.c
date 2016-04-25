#include "mapped_file.h"
#include "../mf/mf.h"
#include <errno.h>
#include <fcntl.h>

mf_handle_t mf_open(const char *pathname){
	return (mf_handle_t) _mf_open(pathname, O_RDWR | O_CREAT, 0755);
}

int mf_close(mf_handle_t mf){
	_mf_close((struct _mf *)mf);
	return 0;
}

ssize_t mf_read(mf_handle_t mf, void *buf, size_t size, off_t offset){
	_mf_seek((struct _mf *)mf, offset);
	return _mf_read((struct _mf *)mf, buf, size);
}

ssize_t mf_write(mf_handle_t mf, const void *buf, size_t size, off_t offset){
	_mf_seek((struct _mf *)mf, offset);
	return _mf_write((struct _mf *)mf, buf, size);
}

//TODO: Rewrite
void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle){
	return _mf_map((struct _mf *)mf, offset, size, mapmem_handle);
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle){
	return _mf_unmap(mapmem_handle);
}

ssize_t mf_file_size(mf_handle_t mf){
	return ((struct _mf *)mf) -> size;
}
