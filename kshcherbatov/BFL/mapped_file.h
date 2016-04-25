// mapped_file.h
// DATE: 23.04.2016

#ifndef __MAPPED_FILE__
#define __MAPPED_FILE__

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef void *mf_handle_t;
#define MF_OPEN_FAILED NULL

typedef void *mf_mapmem_handle_t;
#define MF_MAP_FAILED  NULL

/* All functions on error return value that indicates failure and set errno appropriately  */

/*
 * Returns NULL on failure.
 */
mf_handle_t mf_open(const char *pathname);

/*
 * Returns 0 on success and -1 on failure
 */
int mf_close(mf_handle_t mf);

/*
 * Returns -1 on failure
 */
ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset);

/*
 * Returns -1 on failure
 */
ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset);

/*
 * Returns NULL on failure
 */
void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle);

/*
 * Returns 0 on success and -1 on failure
 */
int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle);

/*
 * Returns -1 on failure
 */
off_t mf_file_size(mf_handle_t mf);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MAPPED_FILE__
