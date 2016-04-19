//
//  mapped_file.h
//  mf_project
//
//  Created by IVAN MATVEEV on 15.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#ifndef mapped_file_h
#define mapped_file_h

#include "hash_table.h"
#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <malloc/malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

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


#ifdef __cplusplus
}
#endif // __cplusplus



#endif /* mapped_file_h */
