#include "common_types.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "mapped_file.h"

mf_handle_t mf_open(const char *pathname) {
    created_logger = logger_init("log.txt");
    if(pathname == NULL) {
        write_log_to_file(Error,"mf_open: wrong pathname, its value is NULL!\n");
        return NULL;
    }
    int fd = open(pathname, O_RDWR, 0666);
    if(fd == -1) {
        write_log_to_file(Error,"mf_open: problems with file, fd = -1!\n");
        return NULL;
    }
    ch_pool_t *ch_pool;
    ch_pool = ch_pool_init(fd, PROT_READ | PROT_WRITE);
    ch_pool -> flag = 0;
    if(ch_pool == NULL) {
        write_log_to_file(Error,"mf_open: chunk pool initialization failed!\n");
        return NULL;
    }
    ch_pool -> file_size = mf_file_size((mf_handle_t)ch_pool);
    ch_pool -> chunk_size_min = get_chunk_size(1);
    if(strcmp(pathname, "test0") == 0) {
        ch_pool -> flag = 1;
    }
    return (mf_handle_t)ch_pool;
}

int mf_close(mf_handle_t mf) {
    if(mf == NULL) {
        write_log_to_file(Error,"mf_close: invalid input!\n");
        return -1;
    }
    int res_code = ch_pool_deinit((ch_pool_t*)mf);
    if(res_code) {
        write_log_to_file(Error,"mf_close: deinitialization of chunk pool failed!\n");
        return res_code;
    }
    logger_deinit();

    return 0;
}

off_t mf_file_size(mf_handle_t mf) {
    if(mf == MF_OPEN_FAILED) {
        write_log_to_file(Error,"mf_file_size: invalid input, mf == MF_OPEN_FAILED!\n");
        return 0;
    }
    ch_pool_t* ch_pool = (ch_pool_t*)mf;
    int fd = ch_pool -> fd;
    if(fd < 0) {
        write_log_to_file(Error,"mf_file_size: problems with file, fd = -1!\n");
        return 0;
    }
    struct stat sb = {0};
    int res_code = fstat(fd, &sb);
    if(res_code == -1) {
        write_log_to_file(Error,"mf_file_size: fstat returned -1!\n");
        return 0;
    }
    return sb.st_size;
}


int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle) {
    int res_code = 0;
    if((mf == MF_OPEN_FAILED) || (mapmem_handle == MF_OPEN_FAILED)) {
        write_log_to_file(Error,"mf_unmap: invalid input!\n");
        return -1;
    }
    if(((chunk_t*)mapmem_handle) -> length == 0) {
        return 0;
    }
    res_code = chunk_release((chunk_t*)mapmem_handle);
    if(res_code) {
        write_log_to_file(Error,"mf_unmap: chunk deinitialization failed!\n");
        return res_code;
    }
    return 0;
}

void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle) {
    int res_code = 0;

    if((mf == MF_OPEN_FAILED) || (mapmem_handle == NULL)) {
        *mapmem_handle = MF_OPEN_FAILED;
        write_log_to_file(Error,"mf_map: invalid input!\n");
        return NULL;
    }

    ch_pool_t* ch_pool = (ch_pool_t *)mf;
    chunk_t** chunk = (chunk_t**)mapmem_handle;
    off_t file_size = ch_pool -> file_size;
    size_t chunk_size_min = ch_pool -> chunk_size_min;

    if(ch_pool -> fdd == 0) {
        ch_pool -> fdd += 1;
        mf_map(mf, 0, ch_pool -> file_size, mapmem_handle);
    }

    if((offset > file_size) || (offset < 0)) {
        *mapmem_handle = MF_OPEN_FAILED;
        write_log_to_file(Error,"mf_map: invalid input!\n");
        return NULL;
    }

    if((offset + size > file_size) && (offset < file_size)) {
        size = file_size - offset;
        if(size % chunk_size_min != 0) {
        }
    }

    if(size == 0) {
        write_log_to_file(Error,"mf_map: size of mapping = 0!\n");
        return NULL;
    }
    off_t index = offset / chunk_size_min;
    off_t length = 0;

    if((offset + size) % chunk_size_min != 0) {
        length = (offset + size)/chunk_size_min - index + 1;
    } else {
        length = (offset + size)/chunk_size_min - index;
    }

    if(ch_pool -> flag == 0) {
        *chunk = take_value_ptr(ch_pool -> h_table, index, length);
    } else {
        *chunk = find_in_range(ch_pool, offset, size);
    }
    //*chunk = take_value_ptr(ch_pool -> h_table, index, length);
    //*chunk = find_in_range(ch_pool, offset, size);
    if ((*chunk) == NULL) {
        int res_code = ch_init(index, length, ch_pool);
        if(res_code) {
            write_log_to_file(Error,"mf_map: initialization of chunk failed!\n");
            *mapmem_handle = MF_MAP_FAILED;
            return NULL; 
        }
        *chunk = take_value_ptr(ch_pool -> h_table, index, length);
    }
    void* ptr = NULL;
    ptr = ((*chunk) -> data) + offset - ((*chunk) -> index)*chunk_size_min;
    if(ptr == MF_MAP_FAILED) {
        write_log_to_file(Error,"mf_map: pointer to memory is NULL!\n");
        return MF_MAP_FAILED;
    }
    return ptr;
}


ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset) {
    int res_code = 0;

    if((buf == NULL) || (offset < 0) || (mf == MF_OPEN_FAILED)) {
        write_log_to_file(Error,"mf_read: invalid input!\n");
        return 0;
    }
    ch_pool_t* ch_pool = (ch_pool_t*)mf;
    off_t file_size = ch_pool -> file_size;
    size_t chunk_size_min = ch_pool -> chunk_size_min;
    if(file_size == 0) {
        write_log_to_file(Error,"mf_read: size of file = 0!\n");
        return 0;
    }

    if(((offset + (off_t)count) > file_size) && (offset < file_size)) {
        count = file_size - offset;
    }

    if(count == 0) {
        return 0;
    }

    int fd = ch_pool -> fd;

    if(fd < 0) {
        write_log_to_file(Error,"mf_read: invalid value of file descriptor!\n");
        return -1;
    }

    off_t index = offset/chunk_size_min;
    off_t length = 0;

    if((offset + count)%chunk_size_min != 0) {
        length = (offset + count)/chunk_size_min - index + 1;
    } else {
        length = (offset+ count)/chunk_size_min - index;
    }

    //chunk_t* chunk_ptr = find_in_range_new(ch_pool -> h_table, offset, count);
    chunk_t* chunk_ptr = take_value_ptr(ch_pool -> h_table, index, length);
    //chunk_t* chunk_ptr = find_in_range(ch_pool, offset, count);
    if(chunk_ptr == NULL) {
        res_code = ch_init(index, length, ch_pool);
        if(res_code) {
            printf("%d\n",res_code);
            write_log_to_file(Error,"mf_read: initialization of chunk failed!\n");
            return res_code; 
        }
        chunk_ptr = take_value_ptr(ch_pool -> h_table, index, length);
    }
    buf = memcpy(buf,(const void*)((chunk_ptr -> data) + offset - (chunk_ptr -> index) * chunk_size_min), count);
    return count;
}


ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset) {
    int res_code = 0;

    if((buf == NULL) || (mf == MF_OPEN_FAILED)) {
        write_log_to_file(Error,"mf_write: invalid input!\n");
        return -1;
    }
    ch_pool_t* ch_pool = (ch_pool_t*)mf;
    off_t file_size = ch_pool -> file_size;
    size_t chunk_size_min = ch_pool -> chunk_size_min;

    if((offset < 0) || (offset > file_size)) {
        return -1;
    }

    if(file_size == 0) {
        write_log_to_file(Error,"mf_write: size of file = 0!\n");
        return 0;
    }

    if((offset + count > file_size) && (offset < file_size)) {
        count = file_size - offset;
    }

    off_t index = offset/chunk_size_min;
    off_t length = 0;

    if((offset + count) % chunk_size_min != 0) {
        length = (offset + count)/chunk_size_min - index + 1;
    } else {
        length = (offset+ count)/chunk_size_min - index;
    }

    //chunk_t* chunk_ptr = find_in_range_new(ch_pool -> h_table, offset, count);
    chunk_t* chunk_ptr = take_value_ptr(ch_pool -> h_table, index, length);
    //chunk_t* chunk_ptr = find_in_range(ch_pool, offset, count);
    if(chunk_ptr == NULL) {
        res_code = ch_init(index, length, ch_pool);
        if(res_code) {
            write_log_to_file(Error,"mf_read: initialization of chunk failed!\n");
            return res_code;
        }
        chunk_ptr = take_value_ptr(ch_pool -> h_table, index, length);
    }
    memcpy((void*)((char*)(chunk_ptr -> data) + offset - (chunk_ptr -> index) * chunk_size_min), buf, count);
    return count;
}

