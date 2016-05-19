//
// Created by voddan on 20/04/16.
//!!
//

#include <printf.h>
#include <file_mapper.h>
#include <key_set.h>

int main(int argn, char ** args) {
    printf("hello File Mapper !\n");

    mf_handle_t mf;
    mf_open("foo", 1, &mf);
    mf_close(mf);

//    key_set* set = set_alloc();
//    set_free(set);

    return 0;
}