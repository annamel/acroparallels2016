#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../sources/mapped_file.h"

#define TESTFAILED 666
#define ERROROCCURED 1337

int testfailed;

void check(int err) {
    err ? printf("FAILED: %d\n", err) : printf("PASSED\n");
    if (err) testfailed = 1;
}

int test_1(const char* filename); // open / close
int test_2(const char* filename); // file size
int test_3(const char* filename); // read
int test_4(const char* filename); // map & unmap
int test_5(const char* filename); // write to a file & read & random access

/* Performance tests */
int ptest_1(const char* filename); // iterate over buffer with big file

int main(int argc, char **argv) {
    testfailed = 0;
    char big[] = "big", small[] = "small";

    printf("Test 01: "); check(test_1(small));
    printf("Test 02: "); check(test_2(small));
    printf("Test 03: "); check(test_3(small));
    printf("Test 04: "); check(test_4(small));
    printf("Test 05: "); check(test_5(small));

    printf("Performance test 01: "); check(ptest_1(big));

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
    if (ptr == NULL || mm_handle == NULL) return ERROROCCURED;

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

int test_5(const char* filename) {
    mf_handle_t handle = mf_open("temp");
    if (mf_write(handle, "oTENCHARSo", 10, 0) == -1) return ERROROCCURED;

    char *buf = (char *)malloc(10);
    memset(buf, 0, 10);
    if (mf_read(handle, buf, 8, 1) == -1) return ERROROCCURED;
    if (strcmp("TENCHARS", buf)) return TESTFAILED;
    if (mf_write(handle, "ELEVEN", 6, 4) == -1) return ERROROCCURED;

    memset(buf, 0, 10);
    if (mf_read(handle, buf, 4, 6) == -1) return ERROROCCURED;
    if (strcmp("EVEN", buf)) return TESTFAILED + 1;

    memset(buf, 0, 10);
    if (mf_read(handle, buf, 6, 1) == -1) return ERROROCCURED;
    if (strcmp("TENELE", buf)) return TESTFAILED + 2;

    free(buf);
    int err = mf_close(handle);
    return err;
}



int ptest_1(const char* filename) {
    mf_handle_t handle = mf_open(filename);
    size_t size = (size_t)mf_file_size(handle);
    size_t bufsize = 1024;
    void *buf = malloc(bufsize);

    int counter = 1337 + 8; // number of ones (1) exists in file
    int counter1 = 1000;
    int counter2 = 300;
    int counter3 = 30;
    int counter4 = 7;
    size_t readbytes = 0;
    char current;
    for (size_t j = 0; readbytes < size; j += bufsize) {
        readbytes += (size_t)mf_read(handle, buf, bufsize, j);
//        printf("buf = %s\n", buf);
        for (int i = 0; i < bufsize; ++i) {
            current = ((char *) buf)[i];
            if (current != '9') counter--;
            if (current == '1') counter1--;
            if (current == '2') counter2--;
            if (current == '3') counter3--;
            if (current == '4') counter4--;
        }
    }
//    printf("\ntotal: %d\n1: %d\n2: %d\n3: %d\n4: %d\n", counter, counter1, counter2, counter3, counter4);
    if (counter || counter1 || counter2 || counter3 || counter4)
        return TESTFAILED;
    free(buf);
    return mf_close(handle);
}
