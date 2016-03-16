#include "logger.h"

struct ring_buff_t *logger;

int main (int argc, char *argv[]) {
    fork(); // it is not essential, you can run simultaneously few copies of programs using this library with 1 logfile

    int pid = getpid();
    printf("%d: JUST TRY KILL THE DAEMON!) You have 10 seconds\n", pid);
    sleep(10);

    LOG_INFO("%d: Hello\n", pid);
    LOG_WARN("%d: it is\n", pid);
    LOG_DEBUG("%d: about\n", pid);
    LOG_ERROR("%d: 5 minuts\n", pid);
    LOG_INFO("%d: before deadline\n", pid);
    for (int i = 0; i < 100; i++)
        LOG_DEBUG("%d: Ura %d\n", pid, i);
    LOG_FATAL("%d: I can't finish tests, but this all working normal in parallel mode!\n", pid);
}