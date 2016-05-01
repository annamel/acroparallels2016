#include "../common_types.h"
#include "../mapped_file.h"
#include "../logger.h"
#include "../list.h"
#include "../hash_table.h"
#include "../chunkmanager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void print_res(int res_code, const char* test_name) {
    if(res_code) {
        printf(test_name);
        printf("error occured!\n");
    } else {
        printf(test_name);
        printf("OK!\n");
    }
}

int test_open_close(const char* f_name) {
    mf_handle_t handle = mf_open(f_name);
    if (handle == NULL){
        return 1;
    }
    return mf_close(handle);
}

int test_file_size(const char* filename) {
    mf_handle_t handle = mf_open(filename);
    ssize_t size = mf_file_size(handle);
    if (size < 0)
    return 1;
    return mf_close(handle);
}

int test_read(const char* filename) {
    mf_handle_t handle = mf_open(filename);
    size_t bytes = mf_file_size(handle)/5;
    off_t offset = mf_file_size(handle)/2;
    void* buf = malloc(bytes*sizeof(void));
    ssize_t result = mf_read(handle, buf, bytes, offset);
    if((result == -1) || (result == 0)) {
        mf_close(handle);
        return 1;
    }
    free(buf);
    return mf_close(handle);
}

int test_map_unmap(const char* filename) {
    mf_handle_t handle = mf_open(filename);
    size_t bytes = mf_file_size(handle)/5;
    off_t offset = mf_file_size(handle)/2;
    void* buf = NULL;
    void* ptr = NULL;
    ptr = mf_map(handle, offset, bytes, &buf);
    if((ptr == NULL) || (buf == NULL)) {
        mf_close(handle);
        return 1;
    }
    int res_code = mf_unmap(handle, buf);
    if(res_code != 0) {
        mf_close(handle);
        return 1;
    }
    return mf_close(handle);
}

int test_read_and_cmp(const char* filename) {
    mf_handle_t handle = mf_open(filename);
    size_t bytes = 11;
    void* buf = NULL;
    off_t offset = 0;
    void* ptr = mf_map(handle, offset, bytes, &buf);
    if((ptr == NULL) || (buf == NULL)) {
        return 1;
    }
    char* buf1 = (char*)calloc(bytes, sizeof(char));
    char* buf2 = (char*)calloc(bytes, sizeof(char));
    if(mf_read(handle, buf1, bytes, offset) == -1) {
        free(buf1);
        free(buf2);
        mf_close(handle);
        return 1;
    }
    memcpy(buf2, ptr, bytes);
    if(memcmp(buf1, ptr, 10) != 0) {
        free(buf1);
        free(buf2);
        mf_close(handle);
        return 1;
    }
    if(mf_unmap(handle, buf)) {
        free(buf1);
        free(buf2);
        mf_close(handle);
        return 1;
    }
    free(buf1);
    free(buf2);
    return mf_close(handle);
}

int test_write_read_cmp(const char* filename) {
    mf_handle_t handle = mf_open(filename);
    size_t bytes = 11;
    void* buf = NULL;
    off_t offset = 40;
    void* ptr = mf_map(handle, offset, bytes, &buf);
    char* buf1 = (char*)calloc(bytes, sizeof(char));
    int i = 0;
    for(i = 0; i < bytes; i++) {
        buf1[i] = 'c';
    }
    if(memcmp(buf1, ptr, bytes) != 0) {
        mf_write(handle, buf1, bytes, offset);
        if(memcmp(buf1, ptr, bytes) == 0){
            mf_close(handle);
            return 0;
        } else {
            mf_close(handle);
            return 1;
        }
    }
    mf_close(handle);
    return 1;
}
/////////////Performance test///////////

int performance_test(const char* filename) {
    mf_handle_t handle = mf_open(filename);
    off_t new_size = (mf_file_size(handle)/get_chunk_size(1))*get_chunk_size(1);
    time_t start, end;
    char* buf = (char*)calloc(2, sizeof(char));
    int i = 0;
    for(i = 0; i < 2; i++) {
        buf[i] = 'A';
    }
    off_t j = 0;
    start = time(NULL);
    if(start == (time_t)-1) {
        exit(EXIT_FAILURE);
    }
    for(j = 0; j < new_size; j++) {
        mf_write(handle, buf, 1,j);
    }
    end = time(NULL);
    printf("\nThe interval was %.2f seconds!\n", difftime(end, start));
    return mf_close(handle);
}


int main(int argc, char*) {

    print_res(test_open_close(argv[1]), "test_open_close: ");
    printf("\n\n");

    print_res(test_file_size(argv[1]), "test_file_size: ");
    printf("\n\n");

    print_res(test_read(argv[1]), "test_read: ");
    printf("\n\n");

    print_res(test_map_unmap(argv[1]), "test_map_unmap: ");
    printf("\n\n");

    print_res(test_read_and_cmp(argv[1]), "test_read_and_cmp: ");
    printf("\n\n");

    print_res(test_write_read_cmp(argv[1]), "test_write_read_cmp: ");
    printf("\n\n");

    print_res(performance_test(argv[1]), "performance_test: ");
    printf("\n\n");

    return 0;
}

