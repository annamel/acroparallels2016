#include "mapped_file.h"
#include "../chunk_manager/chunk_manager.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define COLOR(x) "\x1B[36m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"
#include <assert.h>
#include <sys/sysinfo.h>
#define MIN(x,y) ((x>y) ? y: x)

struct _mf {
	int fd;
	struct chunk_manager cm;
	long int size;
	struct chunk *prev_ch;
};

off_t fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
};

mf_handle_t mf_open(const char *pathname){
	LOG(INFO, "mf_open called\n");
	assert(pathname);

	int fd = open(pathname, O_RDWR | O_CREAT, 0755);
	if (fd == -1)
		return NULL;
	long int size = fsize(pathname);
	if (size == -1)
		return NULL;
	struct _mf *mf = (struct _mf *) malloc(sizeof(struct _mf));
	mf -> fd = fd;
	mf -> size = size;
	mf -> prev_ch = NULL;
	chunk_manager_init(&mf -> cm, fd, O_RDWR | O_CREAT);

	struct sysinfo info; //I know :)
	if (sysinfo(&info) == 0) {
		int tmp;
		struct chunk *ch = NULL;
		chunk_manager_offset2chunk(&mf -> cm, 0, info.freeram / 2, &ch, &tmp, 0);
		mf -> prev_ch = ch;
	}
	return (mf_handle_t) mf;
}

int mf_close(mf_handle_t mf){
	LOG(INFO, "mf_close called\n");
	assert(mf);

	struct _mf * _mf = (struct _mf *) mf;
  	chunk_manager_finalize(&_mf -> cm);
	close(_mf -> fd);
	free(_mf);
	return 0;
}

ssize_t mf_read(mf_handle_t mf, void *buf, size_t size, off_t offset){
	struct _mf * _mf = (struct _mf *) mf;

	LOG(INFO, "mf_read called to read %d bytes by offt %d\n", size, offset);
	if (_mf -> size < offset)
		return 0;
	size = MIN(size, _mf -> size - offset);

  	if (_mf -> size <= offset)
		offset = _mf -> size;
	struct chunk *ch = _mf -> prev_ch;
	int ch_offset = 0;
	ssize_t read_bytes = 0;
	if (ch){
		int ch_size = ch -> length;
		ch_offset = ch -> offset;
		LOG(DEBUG, "Got prev chunk of size %d\n", ch_size);
		if (ch_offset <= offset && offset < ch_offset + ch_size){
			ch_offset = offset - ch_offset;
			long int av_chunk_size = ch_size - ch_offset;
			LOG(DEBUG, "Using chunk prev chunk of av_size %d\n", av_chunk_size);
			long int read_size = MIN(av_chunk_size, size);
			chunk_cpy_c2b(buf, ch, read_size, ch_offset);
			buf += read_size;
			offset += read_size;
			read_bytes += read_size;

			size -= read_size;
		}
	}

	while (size > 0) {
		long int av_chunk_size = chunk_manager_offset2chunk(&_mf -> cm, offset, size, &ch, &ch_offset, 0);
		LOG(DEBUG, "Got chunk of size %d\n", av_chunk_size);
		long int read_size = MIN(av_chunk_size, size);
		chunk_cpy_c2b(buf, ch, read_size, ch_offset);
		buf += read_size;
		offset += read_size;
		read_bytes += read_size;

		if (size <= read_size)
			break;
		size -= read_size;
	}
	_mf -> prev_ch = ch;
	LOG(DEBUG, "End offset is %d\n", offset);
	return read_bytes;
}

ssize_t mf_write(mf_handle_t mf, const void *buf, size_t size, off_t offset){
	struct _mf * _mf = (struct _mf *) mf;
  	LOG(INFO, "mf_write called\n");
	struct chunk *ch = NULL;
	int ch_offset = 0;
	ssize_t write_bytes = 0;
	while (size > 0) {
		long int av_chunk_size = chunk_manager_offset2chunk(&_mf -> cm, offset, size, &ch, &ch_offset, 0);
		LOG(DEBUG, "Got chunk of size %d\n", av_chunk_size);
	  	long int write_size = MIN(av_chunk_size, size);
		LOG(DEBUG, "Stretching file to %d\n", offset + write_size);
		if (lseek(_mf -> fd, offset + write_size - 1, SEEK_SET) == -1) {
        		LOG(ERROR, "Failed file stretch - lseek\n");
			return write_bytes;
		}
		if (write(_mf -> fd, "", 1) == -1){
			LOG(ERROR, "Failed file stretch - write\n");
        		return write_bytes;
		}

		chunk_cpy_b2c(ch, buf, write_size, ch_offset);
		offset += write_size;
		write_bytes += write_size;

		if (size <= av_chunk_size)
			break;
		size -= write_bytes;
	}
	LOG(DEBUG, "End offset is %d\n", offset);
	return write_bytes;
}

void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle){
	LOG(INFO, "mf_map called\n");

	struct _mf * _mf = (struct _mf *) mf;
	if (offset + size > _mf -> size){
		errno = EINVAL;
		return NULL;
	}
	//TODO: Optimization?
  	struct chunk *ch = _mf -> prev_ch;
	int ch_offset = 0;
  	if (ch && ch -> offset <= offset && offset < ch -> offset + ch -> length){
		ch_offset = offset - ch -> offset;
	}else{
		long int av_chunk_size = chunk_manager_offset2chunk(&_mf -> cm, offset, size, &ch, &ch_offset, 1);
		if (av_chunk_size == -1)
			return NULL;
	}

	ch -> ref_cnt++;
	*mapmem_handle = ch;
	return ch -> addr + ch_offset;
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle){
	LOG(INFO, "mf_unmap called\n");

	((struct chunk *)mapmem_handle) -> ref_cnt--;
	return 0;
}

ssize_t mf_file_size(mf_handle_t mf){
	LOG(INFO, "mf_file_size called\n");

	return ((struct _mf *)mf) -> size;
}
