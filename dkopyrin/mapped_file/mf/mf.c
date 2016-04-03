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

struct mf *mf_open(const char *name, int flags, ...){
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
	struct mf *mf = (struct mf *) malloc(sizeof(struct mf));
	mf -> offset = 0;
	mf -> fd = fd;
	mf -> size = size;
	mf -> flags = flags;
	chunk_manager_init(&mf -> cm, mf -> fd, mf -> flags);
	return mf;
}
void mf_close(struct mf *mf) {
	assert(mf);
	chunk_manager_finalize(&mf -> cm);
	close(mf -> fd);
	free(mf);
}

int mf_seek(struct mf *mf, long int offset){
	//TODO: Some checks
	if (offset > mf -> size)
		return -1;
	mf -> offset = offset;
	return 0;
}
long int mf_tell(struct mf *mf){
	return mf -> offset;
}
ssize_t mf_read(struct mf *mf, void *buf, size_t nbyte){
	LOG(INFO, "mf_read called\n");
	nbyte = MIN(nbyte, mf -> size - mf -> offset);
	struct chunk *ch = NULL;
	int ch_offset = 0;
	ssize_t read_bytes = 0;
	while (nbyte >= 0) {
		long int av_chunk_size = chunk_manager_offset2chunk(&mf -> cm, mf -> offset, nbyte, &ch, &ch_offset);
		LOG(DEBUG, "Got chunk of size %d\n", av_chunk_size);
		long int read_size = MIN(av_chunk_size, nbyte);
		LOG(DEBUG, "Stretching file to %d\n", mf -> offset + read_size);
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
ssize_t mf_write(struct mf *mf, void *buf, size_t nbyte){
	LOG(INFO, "mf_write called\n");
	struct chunk *ch = NULL;
	int ch_offset = 0;
	ssize_t write_bytes = 0;
	while (nbyte >= 0) {
		long int av_chunk_size = chunk_manager_offset2chunk(&mf -> cm, mf -> offset, nbyte, &ch, &ch_offset);
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
