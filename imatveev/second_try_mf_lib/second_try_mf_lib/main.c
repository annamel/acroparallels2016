//
//  main.c
//  second_try_mf_lib
//
//  Created by IVAN MATVEEV on 22.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#include <stdio.h>
#include "mapped_file.h"
#include <errno.h>
#include <string.h>

int main(int argc, const char * argv[]) {
    mf_handle_t file = mf_open("/Users/ivanmatveev/Documents/programming/tmp/test.c");
    if (file == MF_OPEN_FAILED){
        printf("%s\n", strerror(errno));
        return -1;
    }
    mf_mapmem_handle_t *mapmem1;
    char *str1 = mf_map(file, 10, 5, mapmem1);
    if (str1 == NULL){
        printf("%s\n", strerror(errno));
        return -1;
    }
    printf("%s\n", str1);
    //sprintf(str1, "MY_FALE>");
    printf("%s\n", str1);
    mf_unmap(file, mapmem1);
    mf_close(file);
    return 0;
}
