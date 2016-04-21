// mapped_file.h
// DATE: 06.04.2016

#ifndef __MAPPED_FILE__
#define __MAPPED_FILE__

#include <errno.h>
#include <sys/types.h>

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
int mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t* mm, void** ptr);
int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mm);


int mf_file_size(mf_handle_t mf, size_t* size);


/* Guaranties that all changes to the file made up to this point are saved to HDD */
int mf_file_sync(mf_handle_t* mf);


#ifdef __cplusplus
}
#endif

#endif // __MAPPED_FILE__
