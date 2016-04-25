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
    void *mem = mf_map(mf, 0, 4096 * 3, &mapmem_handle);
    mem = mf_map(mf, 0, 4096 * 3, &mapmem_handle);
    mem = mf_map(mf, 4096, 4096 * 2, &mapmem_handle);
    mem = mf_map(mf, 2*4096, 4096, &mapmem_handle);
    mem = mf_map(mf, 2*4096 + 2048, 2048, &mapmem_handle);
    mf_unmap(mf, mapmem_handle);
    mf_unmap(mf, mapmem_handle);
    mf_unmap(mf, mapmem_handle);
    mf_unmap(mf, mapmem_handle);
    mf_unmap(mf, mapmem_handle);


    mem = mf_map(mf, 2*4096 + 2048, 4096*2, &mapmem_handle);
    mf_unmap(mf, mapmem_handle);
    mem = mf_map(mf, 3*4096, 4096, &mapmem_handle);
    mf_unmap(mf, mapmem_handle);
    mem = mf_map(mf, 0, 4096 * 3, &mapmem_handle);
    mf_unmap(mf, mapmem_handle);

    char buff[192213] = {0};
    mf_read(mf, buff, mf_file_size(mf), 0);
    write(STDOUT_FILENO, buff, (size_t)mf_file_size(mf));

    mf_close(mf);
    return 0;
}