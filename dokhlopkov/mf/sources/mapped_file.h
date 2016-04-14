// mapped_file.h
// DATE: 06.04.2016

#ifndef __MAPPED_FILE__
#define __MAPPED_FILE__

#include <errno.h>

#define MF_SUCCESS			0
#define MF_OUT_OF_RANGE		1

#define MF_INTERNAL_ERROR	-1
#define MF_OPEN_FAILED		-2
#define MF_INVALID_VALUE	-3
#define MF_NO_MEMORY		ENOMEM
#define MF_FILE_BUSY		EBUSY
#define MF_POOL_FULL		-6
#define MF_ALREADY_RELEASED	-7
#define MF_READ_ONLY		-8
//	MF_INTERNAL_ERROR, MF_FILE_BUSY and MF_NO_MEMORY may result in undefined behaviour
//	Any function might return MF_INTERNAL_ERROR or MF_NO_MEMORY

#ifdef __cplusplus
extern "C" {
#endif

/* Identifies an open file. Valid till `mf_close` invoked. */
typedef void* mf_handle_t;

/* Identifies a mapped buffer. Valid till `mf_unmap` invoked. Unique to the file. */
typedef void* mf_mapmem_handle_t;

/* `mf_close` guaranties that all changes are applied to HDD */
int mf_open(const char* name, size_t max_memory, mf_handle_t* mf);
int mf_close(mf_handle_t mf);

/* Copies data. Buffer `buf` may be used in any way without consequences. */
int mf_read(mf_handle_t mf, off_t offset, size_t size, void* buf);
int mf_write(mf_handle_t mf, off_t offset, size_t size, void* buf);

/*
 * A mapped memory is identified the file `mf` and mapmem handle `mm`.
 * This is similar to file descriptors which are plain `int` numbers.
 * Pointer `ptr` is the start of a buffer where the mapped memory can be accessed
 * */
int mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t* mm, void* ptr);
int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mm);

int mf_file_size(mf_handle_t mf, size_t* size);

#ifdef __cplusplus
}
#endif

#endif // __MAPPED_FILE__
