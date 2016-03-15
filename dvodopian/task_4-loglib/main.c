//
// Created by voddan on 15/03/16.
//

#include <stdio.h>
#include "loglib.h"

int main(int argn, char ** args) {
    printf("hello world\n");


    log_init(256, DEBUG);


    log_write(DEBUG, "hello world!");

    log_flush();

    log_write(INFO, "paparem!");
    log_write(WARN, "hello should not do that!!");

    log_write(ERROR, "that's a trap!");

    log_set_level(WARN);

    log_write(DEBUG, "you can't see me!");

    return 0;
}