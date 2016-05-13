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

/* Performance tests */
int ptest_1(const char* filename); // iterate over buffer with big file

int main(int argc, char **argv) {
    testfailed = 0;
    char big[] = "big";

    printf("Performance test 01: "); check(ptest_1(big));

    return testfailed;
}

int ptest_1(const char* filename) {
    mf_handle_t handle = mf_open(filename);
    size_t size = (size_t)mf_file_size(handle);
    size_t bufsize = 32768;
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
