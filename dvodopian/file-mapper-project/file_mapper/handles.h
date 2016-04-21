//
// Created by voddan on 21/04/16.
//

#ifndef FILE_MAPPER_PROJECT_HANDLES_H
#define FILE_MAPPER_PROJECT_HANDLES_H


typedef struct {
    long            sys_page_size;
    size_t          sys_max_memory;

    char const *    file_name;
    int             file_descriptor;
    off_t           file_size;
} handle;


typedef struct {
    handle * file;
    size_t size;
    off_t offset;

    void * ptr;
} map_handle;


#endif //FILE_MAPPER_PROJECT_HANDLES_H
