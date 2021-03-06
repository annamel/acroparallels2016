#include "../../../include/mapped_file.h"
#include "../local_chunk_manager/local_chunk_manager.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#undef DEBUG_LOG
#define COLOR(x) "\x1B[36m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"
#include <assert.h>
#include <sys/sysinfo.h>
#define MIN(x,y) ((x>y) ? y: x)

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>

//We will not use previous chunk every time because it is ref_cnt sub/add is
//bottleneck. So let's make new chunk with probability 1/USE_PROB
//657
#define USE_PROB 100

struct _mf {
	int fd;
	size_t size;
	struct local_chunk_manager lcm;
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
	if(local_chunk_manager_init(&mf -> lcm, fd, O_RDWR | O_CREAT)){
		free(mf);
		return NULL;
	}

	struct rlimit rl;
	struct sysinfo sys;
	if (!getrlimit(RLIMIT_AS, &rl) && !sysinfo(&sys)) {
		off_t tmp;
		struct chunk *ch = NULL;
		if (rl.rlim_cur == RLIM_INFINITY)
			chunk_manager_gen_chunk(&mf -> lcm.cm, 0, sys.freeram / 2, &ch, &tmp);
		else
			chunk_manager_gen_chunk(&mf -> lcm.cm, 0, MIN(rl.rlim_cur, sys.freeram) / 2, &ch, &tmp);
	}
	//TODO: Logics on setting chunks
	//int thr;
	//for (thr = 0; thr < MAX_NUM_THREADS; thr++)
	//	mf -> prev_ch_by_thread[thr] = ch;

	return (mf_handle_t) mf;
}

int mf_close(mf_handle_t mf){
	LOG(INFO, "mf_close called\n");
	assert(mf);

	struct _mf * _mf = (struct _mf *) mf;
  	local_chunk_manager_finalize(&_mf -> lcm);
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


ssize_t mf_iterator(struct local_chunk_manager* lcm, off_t offset,
                    size_t size, void *buf, void (*itfunc)(struct chunk *, size_t, off_t, void *)){
	struct chunk *ch = NULL;
	ssize_t read_bytes = 0;

	struct local_chunk * local_ch;
	//Nearly the same approach is used here for getting new chunk
	off_t ch_offset = 0;
	size_t av_chunk_size = local_chunk_manager_gen_chunk(lcm, offset, size, &local_ch, &ch_offset);
	ch = local_ch -> chunk;
	if (!ch)
		return read_bytes;
	LOG(DEBUG, "Got chunk of size %d\n", av_chunk_size);
	size_t read_size = MIN(av_chunk_size, size);
	itfunc(ch, read_size, ch_offset, buf);
	read_bytes += read_size;

       return read_bytes;
}

ssize_t mf_read(mf_handle_t mf, void *buf, size_t size, off_t offset){
	LOG(INFO, "mf_read called to read %d bytes by offt %d\n", size, offset);
	assert(mf); assert(buf);
	struct _mf * _mf = (struct _mf *) mf;

	size = MIN(size, _mf -> size - offset);
  	if (_mf -> size <= offset)
		return 0;
	return mf_iterator(&_mf -> lcm, offset, size, buf, mf_read_itfunc);
}

ssize_t mf_write(mf_handle_t mf, const void *buf, size_t size, off_t offset){
	LOG(INFO, "mf_write called\n");
	assert(mf); assert(buf);
	struct _mf * _mf = (struct _mf *) mf;
	return mf_iterator(&_mf -> lcm, offset, size, (void *)buf, mf_write_itfunc);
}

void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle){
	LOG(INFO, "[%d] mf_map called, off %ld, size %ld\n", GET_THREAD_INDEX(), offset, size);
	assert(mf); assert(mapmem_handle);

	struct _mf * _mf = (struct _mf *) mf;
	if (offset + size > _mf -> size){
		LOG(DEBUG, "Bad inval\n");
		errno = EINVAL;
		return NULL;
	}

	struct local_chunk * local_ch;
	off_t ch_offset;
	size_t av_chunk_size = local_chunk_manager_gen_chunk(&_mf -> lcm, offset, size, &local_ch, &ch_offset);
	if (av_chunk_size == -1){
		errno = EAGAIN;
		return NULL;
	}

	struct chunk *ch = local_ch -> chunk;
	// We use chunk as mapmem handle because we only need to decrease ref_cnt
	// when unmap is called

	local_ch -> local_ref_cnt++;
	*mapmem_handle = local_ch;
	return ch -> addr + ch_offset;
}

int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle){
	LOG(INFO, "mf_unmap called\n");
	assert(mf); assert(mapmem_handle);

	((struct local_chunk *)mapmem_handle) -> local_ref_cnt--;
	return 0;
}

ssize_t mf_file_size(mf_handle_t mf){
	LOG(INFO, "mf_file_size called\n");
	assert(mf);

	struct _mf * _mf = (struct _mf *) mf;
	//We believe that filesize is not changed during program flow
	return _mf -> size;
}
