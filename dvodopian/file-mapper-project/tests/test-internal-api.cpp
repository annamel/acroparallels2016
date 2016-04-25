//
// Created by voddan on 20/04/16.
//

#include <gtest/gtest.h>
#include <fcntl.h>
#include "../file_mapper/file_mapper.h"

char test_file_small[] = "/Users/voddan/Programming/Parallels/file-mapper-project/tests/file_1.5Mb.mp4";
char test_file_big[] = "/Users/voddan/Programming/Parallels/file-mapper-project/tests/file_23Mb.mp4";
char test_file_huge[] = "/Users/voddan/Programming/Parallels/file-mapper-project/tests/file_1.6Gb.mkv";

TEST(InternalAPI, TestFiles) {
    EXPECT_EQ(0, access(test_file_small, R_OK|W_OK));
    EXPECT_EQ(0, access(test_file_big,   R_OK|W_OK));
    EXPECT_EQ(0, access(test_file_huge,  R_OK|W_OK));
}

TEST(InternalAPI, OpenCloseFile) {
    mf_handle_t mf;
    mf_open(test_file_small, 1024, &mf);
    mf_close(mf);
}

TEST(InternalAPI, MapRW) {
    mf_handle_t mf;
    mf_open(test_file_small, 1024, &mf);

    mf_mapmem_handle_t mm;
    void* ptr;
    mf_map(mf, 0, 100, &mm, &ptr);
    char * arr = (char *) ptr;


    char old[10];
    for (int i = 0; i < 10; ++i) {
        old[i] = arr[i];
    }
    for (int i = 0; i < 10; ++i) {
        arr[i] = (char) (i + 10);
    }

    mf_unmap(mf, mm);
    mf_map(mf, 0, 100, &mm, &ptr);
    arr = (char *) ptr;
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(i + 10, arr[i]);
    }
    mf_unmap(mf, mm);

    mf_close(mf);
}
