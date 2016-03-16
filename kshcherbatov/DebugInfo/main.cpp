#include "logger.h"
#include <unistd.h>
#include <stdio.h>

struct ring_buff_t *logger;

int main (int argc, char *argv[]) {
    fork(); // it is not essential, you can run simultaneously few copies of programs using this library with 1 logfile

    printf("JUST TRY KILL THE DAEMON!) You have 10 seconds\n");
    //sleep(10);

    LOG_INFO("Hello\n");
    LOG_WARN("it is\n");
    LOG_DEBUG("about\n");
    LOG_ERROR("5 minuts\n");
    LOG_INFO("before deadline\n");
    for (int i = 0; i < 100; i++)
        LOG_DEBUG("Ura %d\n", i);

    LOG_FATAL("I can't finish tests, but this all working normal in parallel mode!\n");
}