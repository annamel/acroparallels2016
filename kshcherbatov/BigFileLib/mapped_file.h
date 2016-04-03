// mapped_file.h
// VERSION 1.0 RC2
// DATE: 20.03.2016

#ifndef __MAPPED_FILE__
#define __MAPPED_FILE__

#if _WIN32 || _WIN64
	#include <BaseTsd.h>
	typedef UINT64 uint64_t;
#else
	#include <stdint.h>
#include <cstddef>

#endif

typedef void* mf_handle_t;

int mf_open(const char *name, size_t pool_size, size_t chunk_size, int read_only, mf_handle_t *mf);
int mf_close(mf_handle_t* mf);
int mf_chunk_unacquire(mf_handle_t *mf, size_t offset);
int mf_read(mf_handle_t *mf, size_t offset, size_t length);

//PROT_WRITE
//MAP_SHARED
/*
 *  #include <sys/mman.h>

       void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);
       int munmap(void *addr, size_t length);

 */
#endif // __MAPPED_FILE__
