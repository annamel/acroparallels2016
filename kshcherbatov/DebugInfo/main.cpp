#include "lmsg_ring_buff.h"

struct ring_buff_t *logger;

int main (int argc, char *argv[]) {
    //TODO: use __attribute__ ((constructor));
    logger = __ring_buff_construct("log.txt");

    LOG_INFO("Hello\n");
    LOG_WARN("it is\n");
    LOG_DEBUG("about\n");
    LOG_ERROR("5 minuts\n");
    LOG_INFO("before deadline");
    for (int i = 0; i < 250; i++)
        LOG_DEBUG("Ura %d\n", i);
    LOG_FATAL("I can't finish tests, but this all working normal in parallel mode!\n");

    __ring_buff_destruct(logger);
}