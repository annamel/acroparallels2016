#include "mf.h"

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>

#define COLOR(x) "\x1B[36m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"

#define MIN(x,y) ((x>y) ? y: x)

off_t fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

struct _mf *_mf_open(const char *name, int flags, ...){
	LOG(INFO, "mf_open called\n");
	assert(name);

	int fd = -1;
	if (flags & O_CREAT) {
		va_list arguments;
		va_start(arguments, flags);
		mode_t mode = va_arg(arguments, mode_t);
		fd = open(name, flags, mode);
		va_end (arguments);
	}else{
		fd = open(name, flags);
	}
	if (fd == -1)
		return NULL;
	long int size = fsize(name);
	if (size == -1)
		return NULL;
	struct _mf *mf = (struct _mf *) malloc(sizeof(struct _mf));
	mf -> offset = 0;
	mf -> fd = fd;
	mf -> size = size;
	mf -> flags = flags;
	chunk_manager_init(&mf -> cm, mf -> fd, mf -> flags);
	return mf;
}
void _mf_close(struct _mf *mf) {
	assert(mf);
	chunk_manager_finalize(&mf -> cm);
	close(mf -> fd);
	free(mf);
}

int _mf_seek(struct _mf *mf, long int offset){
	//TODO: Some checks
	//if (offset > mf -> size)
	//	return -1;
	mf -> offset = offset;
	return 0;
}
long int _mf_tell(struct _mf *mf){
	return mf -> offset;
}
ssize_t _mf_read(struct _mf *mf, void *buf, size_t nbyte){
	LOG(INFO, "mf_read called\n");
	if (mf -> size <= mf -> offset)
		mf -> offset = mf -> size;
	nbyte = MIN(nbyte, mf -> size - mf -> offset);
	struct chunk *ch = NULL;
	int ch_offset = 0;
	ssize_t read_bytes = 0;
	while (nbyte >= 0) {
		long int av_chunk_size = chunk_manager_offset2chunk(&mf -> cm, mf -> offset, nbyte, &ch, &ch_offset, 0);
		LOG(DEBUG, "Got chunk of size %d\n", av_chunk_size);
		long int read_size = MIN(av_chunk_size, nbyte);
		chunk_cpy_c2b(buf, ch, read_size, ch_offset);
		buf += read_size;
		mf -> offset += read_size;
		read_bytes += read_size;

		if (nbyte <= read_size)
			break;
		nbyte -= av_chunk_size;
	}
	LOG(DEBUG, "End offset is %d\n", mf -> offset);
	return read_bytes;
}
ssize_t _mf_write(struct _mf *mf, const void *buf, size_t nbyte){
	LOG(INFO, "mf_write called\n");
	struct chunk *ch = NULL;
	int ch_offset = 0;
	ssize_t write_bytes = 0;
	while (nbyte >= 0) {
		long int av_chunk_size = chunk_manager_offset2chunk(&mf -> cm, mf -> offset, nbyte, &ch, &ch_offset, 0);
		LOG(DEBUG, "Got chunk of size %d\n", av_chunk_size);
	  	long int write_size = MIN(av_chunk_size, nbyte);
		LOG(DEBUG, "Stretching file to %d\n", mf -> offset + write_size);
		if (lseek(mf -> fd, mf -> offset + write_size - 1, SEEK_SET) == -1) {
        		LOG(ERROR, "Failed file stretch - lseek\n");
			return write_bytes;
		}
		if (write(mf -> fd, "", 1) == -1){
			LOG(ERROR, "Failed file stretch - write\n");
        		return write_bytes;
		}

		chunk_cpy_b2c(ch, buf, write_size, ch_offset);
		mf -> offset += write_size;
		write_bytes += write_size;

		if (nbyte <= av_chunk_size)
			break;
		nbyte -= av_chunk_size;
	}
	LOG(DEBUG, "End offset is %d\n", mf -> offset);
	return write_bytes;
}

void *_mf_map(struct _mf *mf, off_t offset, size_t size, void ** mh){
  	struct chunk *ch = NULL;
	int ch_offset = 0;
	long int av_chunk_size = chunk_manager_offset2chunk(&mf -> cm, offset, size, &ch, &ch_offset, 1);
	if (av_chunk_size == -1)
		return (void *) -1;

	ch -> ref_cnt++;
	*mh = ch;
	return ch -> addr + ch_offset;
}
int _mf_unmap(void *mm){
	((struct chunk *)mm) -> ref_cnt--;
	return 0;
}
