#include "mapped_file.h"
#include <stdlib.h>
#include "logger.h"
#include <unistd.h>
#include <sys/stat.h>
//int mf_open(const char *name, unsigned pool_size, uint64_t chunk_std_size, int read_only, mf_handle_t *mf);


int main() {

    //#define LOG_FATAL(fmt, args...) ring_buf_push_msg(Logging_system, LOGLEVEL_FATAL, getpid(), fmt, ##args)

    mf_handle_t mf;
    mf_open("logger.cpp", (uint64_t)(sysconf(_SC_PHYS_PAGES) >> 2), 4096*4, false, &mf);
    if (!mf) {
        LOG_ERROR("mf_open constructor failed.\n", NULL);
        return EXIT_FAILURE;
    }

    mf_read(&mf, 0*sizeof('/'), 1000000000);

    mf_close(&mf);
    return 0;
}