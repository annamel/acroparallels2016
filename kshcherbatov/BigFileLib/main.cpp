#include "mapped_file.h"
#include <stdlib.h>
#include "logger.h"
#include <unistd.h>
#include <sys/stat.h>

int main() {
    mf_handle_t mf;
    mf_open("logger.cpp", 3, 0, false, &mf);
    if (!mf) {
        LOG_ERROR("mf_open constructor failed.\n", NULL);
        return EXIT_FAILURE;
    }

    mf_read(&mf, 0*sizeof('/'), 1000000000);

    mf_close(&mf);
    return 0;
}