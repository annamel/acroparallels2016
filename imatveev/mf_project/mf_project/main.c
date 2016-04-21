//
//  main.c
//  mf_project
//
//  Created by IVAN MATVEEV on 15.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#include <stdio.h>
#include "list.h"
#include "mapped_file.h"
#include "hash_table.h"

int main(int argc, const char * argv[]) {
    errno = 0;
    mf_handle_t file = mf_open("/Users/ivanmatveev/Desktop/speech_2.pages", 0);
    printf("file:%p\n errno = %d\n", file, errno);
    size_t size_buf = 100;
    unsigned char *buf = malloc(size_buf*sizeof(char));
    if (buf == NULL) {
        printf("error\n");
        return 0;
    }
    int i;
    ssize_t ret;
    for (i = 0; i < size_buf; i++)
        buf[i] = 'a';
    errno = 0;
    if ((ret = mf_read(file, 0, size_buf, buf)) < 0){
        printf("error in read with value %zd\nerrno = %d\n", ret, errno);
        printf("%s", strerror(errno));
    } else {
        printf("ret = %zd\n", ret);
        printf("read from file:\n");
        for (i = 0; i < size_buf; i++)
            printf("0x%02x %c ", buf[i], buf[i]);
        printf("\n");
    }
    free(buf);
    return 0;
}
