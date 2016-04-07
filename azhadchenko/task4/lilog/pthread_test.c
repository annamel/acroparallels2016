#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <execinfo.h>
#include <pthread.h>

#define LOG_LEVEL_3

#include "logger.h"


void call3() {
    _if(0)
        return;
};

void call2() {
    call3();
};

void call1() {
    call2();
};

void call0() {
    call1();
};

struct TestStruct {
    pthread_t id;
    int num;
};

void* pthread_test_func(void* arg) {


    struct TestStruct* item = (struct TestStruct*)arg;

    for(int i = 0; i < item -> num; i++)
        lilog(i % 5, "Thread %d msg %d", item -> num, i);

    if(item -> num == 1)
        call0();

    return 0;
}

int main(int argc, char** argv)
{
    init_logger(argv, argv[1]);

    lilog(INFO, "Such log");
    lilog(DEBUG, "So useful");
    lilog(INFO, "SUCH DATA %d", getpid());
    lilog_cond(0 < -3);
    lilog_cond_msg(!!0, "wow %s", "WOW");


    lilog_print_stack("Such stack");

#define THREAD_NUM 10

    struct TestStruct* tinfo = (struct TestStruct*)calloc(sizeof(struct TestStruct), THREAD_NUM);

    for(int i = 0; i < THREAD_NUM; i++) {
        tinfo[i].num = i;
        pthread_create(&tinfo[i].id, NULL, &pthread_test_func, &tinfo[i]);
    }

    for(int i = 0; i < THREAD_NUM; i++)
        pthread_join(tinfo[i].id, NULL);

    lilog_finish(argv);


    free(tinfo);

    return 0;
}

