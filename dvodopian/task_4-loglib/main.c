//
// Created by voddan on 15/03/16.
//

#include <stdio.h>
#include "loglib.h"

void somefunc() {
    log_write(WARN, "hello should not do that!!");
    log_write(ERROR, "that's a trap!");

}

int main(int argn, char ** args) {
    printf("hello world\n");

//    log_init(stderr, 256, DEBUG);
//    flogger("log", DEBUG);
    logger(DEBUG);

    log_write(DEBUG, "hello world!");

    log_flush();

    log_write(INFO, "paparem!");


    somefunc();

    log_write(DEBUG, "yups");


    log_set_level(WARN);

    log_write(DEBUG, "you can't see me!");

    log_write(WARN, "I am after some DEBUG");

//    log_flush();

    return 0;
}

