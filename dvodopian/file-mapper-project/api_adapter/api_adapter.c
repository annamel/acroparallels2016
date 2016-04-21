//
// Created by voddan on 20/04/16.
//

#include "api_adapter.h"


mf_handle_t mf_open(const char* pathname, size_t max_memory_usage) {
    return 0;
}


int mf_close(mf_handle_t mf) {
    return 0;
}


ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void* buf) {
    return 0;
}


ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void* buf) {
    return 0;
}


mf_mapmem_t* mf_map(mf_handle_t mf, off_t offset, size_t size) {
    return 0;
}


int mf_unmap(mf_mapmem_t* mm) {
    return 0;
}


ssize_t mf_file_size(mf_handle_t mf) {
    return 0;
}