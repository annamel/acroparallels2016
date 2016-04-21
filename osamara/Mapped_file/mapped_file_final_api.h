// mapped_file.h
// DATE: 10.04.2016

#ifndef __MAPPED_FILE__
#define __MAPPED_FILE__
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// ENOTSUP and ENOMEM may result in undefined behaviour

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/* Identifies an open file */

typedef void* mf_handle_t;

#define MF_OPEN_FAILED NULL

/* Identifies a mapped memory */

typedef void* mf_mapmem_handle_t;

typedef struct mf_mapped_memory
{
	void* ptr;
	mf_mapmem_handle_t handle;

} mf_mapmem_t;

#define MF_MAP_FAILED NULL

/* All functions on error return value that indicates failure and set errno appropriately  */

/*
 * 'max_memory_usage' is an optional parameter, pass '0' if you want default value.
 * Returns NULL on failure.
 */
mf_handle_t mf_open(const char* pathname, size_t max_memory_usage);

/*
 * Returns 0 on success and -1 on failure
 */
int mf_close(mf_handle_t mf);

/*
 * Returns -1 on failure
 */
ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void* buf);

/*
 * Returns -1 on failure
 */
ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void* buf);

/*
 * Returns NULL on failure
 */
mf_mapmem_t* mf_map(mf_handle_t mf, off_t offset, size_t size);

/*
 * Returns 0 on success and -1 on failure
 */
int mf_unmap(mf_mapmem_t* mm);

/*
 * Returns -1 on failure
 */
ssize_t mf_file_size(mf_handle_t mf);

struct my_mf_handle_t {
	int fd;
};
struct my_mf_mapmem_handle_t{
	off_t offset;
	size_t size;
};

typedef struct my_mf_handle_t my_mf_handle_t;
typedef struct my_mf_mapmem_handle_t my_mf_mapmem_handle_t;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MAPPED_FILE__
