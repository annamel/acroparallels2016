//
// Created by voddan on 07/05/16.
//

#ifndef FILE_MAPPER_PROJECT_CHUNK_MANAGER_H
#define FILE_MAPPER_PROJECT_CHUNK_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/mman.h>

typedef struct chman_slice chman_slice;

typedef struct chunk_manager chunk_manager;

typedef unsigned int chman_index;


typedef struct {
    char const *    file_name;
    int             file_descriptor;
    off_t           file_size;
} file_info;


chunk_manager* chman_alloc(file_info * info, off_t page_size);

void chman_free(chunk_manager* chm);

void* chman_slice_ptr(chman_slice* s);


chman_slice* chman_up_range(chunk_manager* chm, chman_index first, int num);

void chman_up_bunch(chunk_manager * chm, chman_index first, int num, chman_slice ** arr);

chman_slice* chman_down(chunk_manager* chm, chman_slice* sl);

#ifdef __cplusplus
}
#endif

#endif //FILE_MAPPER_PROJECT_CHUNK_MANAGER_H
