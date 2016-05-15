#include "../../../include/mapped_file.h"
#include "../chunk_manager/chunk_manager.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>

#define COLOR(x) "\x1B[36m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"
#include <assert.h>
#include <sys/sysinfo.h>
#define MIN(x,y) ((x>y) ? y: x)

struct _mf {
	int fd;
	size_t size;
	struct chunk_manager cm;
	struct chunk *prev_ch;
};

size_t fsize(const char *filename) {
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
	size_t size = fsize(pathname);
	if (size == -1)
		return NULL;
	struct _mf *mf = (struct _mf *) malloc(sizeof(struct _mf));
	if (mf == NULL)
		return NULL;
	mf -> fd = fd;
	mf -> size = size;
	mf -> prev_ch = NULL;
	if(chunk_manager_init(&mf -> cm, fd, O_RDWR | O_CREAT)){
		free(mf);
		return NULL;
	}

	/*
	 * We try to mmap part of file to improve performance. In order to do
	 * this we must have enough virtual memory that is checked by getrlimit
	 * function.
	 */

	struct rlimit rl;
	struct sysinfo sys;
	if (!getrlimit(RLIMIT_AS, &rl) && !sysinfo(&sys)) {
		off_t tmp;
		struct chunk *ch = NULL;
		if (rl.rlim_cur == RLIM_INFINITY)
			chunk_manager_gen_chunk(&mf -> cm, 0, sys.freeram / 2, &ch, &tmp);
		else
			chunk_manager_gen_chunk(&mf -> cm, 0, MIN(rl.rlim_cur / 2, sys.freeram / 2), &ch, &tmp);
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

#ifdef MEMORY_DEBUG
	_mf -> fd = 0xDEAD;
	_mf -> size = 0xDEADBEEF;
#endif
  	free(_mf);
	return 0;
}


void mf_read_itfunc(struct chunk *ch, size_t ch_size, off_t ch_offset, void *buf){
	chunk_cpy_c2b(buf, ch, ch_size, ch_offset);
}

void mf_write_itfunc(struct chunk *ch, size_t ch_size, off_t ch_offset, void *buf){
	chunk_cpy_b2c(ch, buf, ch_size, ch_offset);
}


ssize_t mf_iterator(struct chunk_manager* cm, struct chunk ** prev_ch, off_t offset,
                    size_t size, void *buf, void (*itfunc)(struct chunk *, size_t, off_t, void *)){
	struct chunk *ch = *prev_ch;
	ssize_t read_bytes = 0;

	//This if tries to use chunk from previous iters: prev_ch
	if (ch){
		size_t ch_size = ch -> size;
		off_t ch_offset = ch -> offset;
		LOG(DEBUG, "Got prev chunk of size %ld\n", ch_size);
		//Check if prev chunk is good for this task: offset lays in chunk
		if (ch_offset <= offset && offset < ch_offset + ch_size){
			/* If we use chunk not from beginning but from relative offset
			 * then we only can read av_chunk_size. We believe, that
			 * itfunc actually read from chunk
			 */
			ch_offset = offset - ch_offset;
		  	size_t av_chunk_size = ch_size - ch_offset;
			LOG(DEBUG, "Using chunk prev chunk of av_size %ld\n", av_chunk_size);
			size_t read_size = MIN(av_chunk_size, size);
			itfunc(ch, read_size, ch_offset, buf);
			buf += read_size;
			offset += read_size;
			read_bytes += read_size;

			size -= read_size;
		}
	}
	LOG(DEBUG, "Left to read %ld\n", size);

	if (size <= 0)
		return read_bytes;

	//Nearly the same approach is used here for getting new chunk
	ch = NULL;
	off_t ch_offset = 0;
	ssize_t av_chunk_size = chunk_manager_gen_chunk(cm, offset, size, &ch, &ch_offset);
	if (av_chunk_size == -1)
		return read_bytes;
	LOG(DEBUG, "Got chunk of size %ld\n", av_chunk_size);
	size_t read_size = MIN(av_chunk_size, size);
	itfunc(ch, read_size, ch_offset, buf);
	read_bytes += read_size;

	//After iterations set new prev chunk
	*prev_ch = ch;
	return read_bytes;
}

ssize_t mf_read(mf_handle_t mf, void *buf, size_t size, off_t offset){
	LOG(INFO, "mf_read called to read %d bytes by offt %d\n", size, offset);
	assert(mf); assert(buf);
	struct _mf * _mf = (struct _mf *) mf;

	size = MIN(size, _mf -> size - offset);
  	if (_mf -> size <= offset)
		return 0;
	return mf_iterator(&_mf -> cm, &_mf -> prev_ch, offset, size, buf, mf_read_itfunc);
}

ssize_t mf_write(mf_handle_t mf, const void *buf, size_t size, off_t offset){
	LOG(INFO, "mf_write called\n");
	assert(mf); assert(buf);
	struct _mf * _mf = (struct _mf *) mf;
	return mf_iterator(&_mf -> cm, &_mf -> prev_ch, offset, size, (void *)buf, mf_write_itfunc);
}

void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle){
	//LOG(INFO, "mf_map called\n");
	assert(mf); assert(mapmem_handle);

	struct _mf * _mf = (struct _mf *) mf;
	if (offset + size > _mf -> size){
		LOG(DEBUG, "Bad inval\n");
		errno = EINVAL;
		return NULL;
	}
	//Map works nearly the same as r/w: firstly we try prev_ch
  	struct chunk *ch = _mf -> prev_ch;
	off_t ch_offset = 0;
  	if (ch && ch -> offset <= offset && offset < ch -> offset + ch -> size){
		//Chunk is OK, we have to set relative chunk offset
		//LOG(DEBUG, "Get chunk from cache\n");
		ch_offset = offset - ch -> offset;
	}else{
		//Elsewhere we generate a new one
		LOG(DEBUG, "Gen new chunk\n");
		size_t av_chunk_size = chunk_manager_gen_chunk(&_mf -> cm, offset, size, &ch, &ch_offset);
		if (av_chunk_size == -1)
			return NULL;
	}

	ch -> ref_cnt++;
	_mf -> prev_ch = ch;
	// We use chunk as mapmem handle because we only need to decrease ref_cnt
	// when unmap is called
	*mapmem_handle = ch;
	return ch -> addr + ch_offset;
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle){
	//LOG(INFO, "mf_unmap called\n");
	assert(mf); assert(mapmem_handle);

	((struct chunk *)mapmem_handle) -> ref_cnt--;
	return 0;
}

ssize_t mf_file_size(mf_handle_t mf){
	LOG(INFO, "mf_file_size called\n");
	assert(mf);

	struct _mf * _mf = (struct _mf *) mf;
	//We believe that filesize is not changed during program flow
	return _mf -> size;
}
