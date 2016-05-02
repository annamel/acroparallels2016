#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../sources/mapped_file.h"

#define TESTFAILED 666
#define ERROROCCURED 1337

int testfailed;

int test_1(const char* filename); // open / close
int test_2(const char* filename); // file size
int test_3(const char* filename); // read
int test_4(const char* filename); // map & unmap

void check(int err) {
    err ? printf("FAILED: %d\n", err) : printf("PASSED\n");
    if (err) testfailed = 1;
}

int main(int argc, char **argv) {
    testfailed = 0;
    char small[] = "small";

    printf("Test 01: "); check(test_1(small));
    printf("Test 02: "); check(test_2(small));
    printf("Test 03: "); check(test_3(small));
    printf("Test 04: "); check(test_4(small));

    return testfailed;
}

int test_1(const char* filename) {
    mf_handle_t handle = mf_open(filename);
    if (handle == NULL) return TESTFAILED;
    return mf_close(handle);
}

int test_2(const char* filename) {
    mf_handle_t handle = mf_open(filename);
    ssize_t size = mf_file_size(handle);
    if (size == -1) return TESTFAILED;
    return mf_close(handle);
}

int test_3(const char* filename) {
    size_t bytes_to_read = 10;
    mf_handle_t handle = mf_open(filename);
    void *buf = malloc(bytes_to_read);
    ssize_t size = mf_file_size(handle);
    ssize_t read_bytes = mf_read(handle, buf, bytes_to_read, mf_file_size(handle) / 2);
    if (read_bytes == -1) return ERROROCCURED;

    if (!((size_t)read_bytes == bytes_to_read || read_bytes == size))
        return (int)(read_bytes - bytes_to_read);
    free(buf);
    return mf_close(handle);
}

int test_4(const char* filename) {
    mf_handle_t handle = mf_open(filename);
    size_t msgsize = 1337;

    mf_mapmem_handle_t mm_handle = NULL;

    void *ptr = mf_map(handle, 0, msgsize, &mm_handle);
    if (ptr == NULL) return ERROROCCURED;

    char *buf = (char *)malloc(msgsize);
    memset(buf, 0, msgsize);
    if (mf_read(handle, buf, msgsize, 0) == -1) return ERROROCCURED;

    char *buff = (char *)malloc(msgsize);
    memcpy(buff, ptr, msgsize);
    if (strcmp(buf, buff)) return TESTFAILED;

    int err = mf_unmap(handle, mm_handle);
    if (err) return err;

    free(buf); free(buff);
    return mf_close(handle);
}
