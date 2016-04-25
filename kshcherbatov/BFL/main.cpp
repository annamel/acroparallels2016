#include "mapped_file.h"
#include <stdlib.h>
#include "logger.h"

int main() {
    mf_handle_t mf = mf_open("gui.cpp");
    if (!mf) {
        LOG_ERROR("mf_open constructor failed.\n", NULL);
        return EXIT_FAILURE;
    }

    LOG_INFO("File size info = %d\n\n", mf_file_size(mf));

    mf_mapmem_handle_t mapmem_handle;
    void *mem;

    mem = mf_map(mf, 0, 4096 * 2 + 2048, &mapmem_handle);
    mf_unmap(mf, mapmem_handle);

    mf_close(mf);
    return 0;
}