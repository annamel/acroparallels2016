#include "mapped_file.h"
#include <stdlib.h>
#include "logger.h"
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include "hash_table.h"

int main() {
    mf_handle_t mf = mf_open("gui.cpp", 0);
    if (!mf) {
        LOG_ERROR("mf_open constructor failed.\n", NULL);
        return EXIT_FAILURE;
    }

    size_t file_size = (size_t)mf_file_size(mf);
    mf_mapmem_t *mapmem = mf_map(mf, 0, file_size);
    write(STDOUT_FILENO, mapmem->ptr, file_size);

    mf_unmap(mapmem);

    mf_close(mf);
    return 0;
}