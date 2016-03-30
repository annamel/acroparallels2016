#include "mf.h"

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#define COLOR(x) "\x1B[36m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"

int mf_open(const char *name, struct mf *mf){
	assert(mf);
	assert(name);
	mf -> offset = 0;
	mf -> fd = open(name, O_RDONLY);
	assert(mf -> fd != -1);
	chunk_manager_init(&mf -> cm, mf -> fd);
	return 0;
}
void mf_close(struct mf *mf) {
	assert(mf);
	chunk_manager_finalize(&mf -> cm);
	close(mf -> fd);
}

int mf_seek(struct mf *mf, long int offset){
	//TODO: Some checks
	mf -> offset = offset;
	return 0;
}
long int mf_tell(struct mf *mf){
	return mf -> offset;
}
ssize_t mf_read(struct mf *mf, void *buf, size_t nbyte){
	LOG(INFO, "mf_read called\n");
	struct chunk *ch = NULL;
	int ch_offset = 0;
	ssize_t read_bytes = 0;
	while (nbyte >= 0) {
		long int av_chunk_size = chunk_manager_offset2chunk(&mf -> cm, mf -> offset, nbyte, &ch, &ch_offset);
		LOG(DEBUG, "Got chunk of size %d\n", av_chunk_size);
		if (av_chunk_size >= nbyte){
			LOG(DEBUG, "That is actually enough for %d\n", nbyte);
			chunk_cpy_c2b(buf, ch, nbyte, ch_offset);
			mf -> offset += nbyte;
			read_bytes += nbyte;
			break;
		}else{
			LOG(DEBUG, "That is not enough for %d\n", nbyte);
			chunk_cpy_c2b(buf, ch, av_chunk_size, ch_offset);
			buf += av_chunk_size;
			mf -> offset += av_chunk_size;
			read_bytes += av_chunk_size;
		}
		nbyte -= av_chunk_size;
	}
	LOG(DEBUG, "End offset is %d\n", mf -> offset);
	return read_bytes;
}
ssize_t mf_write(struct mf *mf, void *buf, size_t nbyte);
