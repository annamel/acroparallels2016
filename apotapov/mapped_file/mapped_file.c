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
   // write_log_to_file(Info,"mf_open: started!\n");
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
    if(ch_pool == NULL) {
        write_log_to_file(Error,"mf_open: chunk pool initialization failed!\n");
        return NULL;
    }
   // write_log_to_file(Info,"mf_open: finished!\n");

    return (mf_handle_t)ch_pool;
}

int mf_close(mf_handle_t mf) {
   // write_log_to_file(Info,"mf_close: started!\n");
    if(mf == NULL) {
        write_log_to_file(Error,"mf_close: invalid input!\n");
        return -1;
    }
    int res_code = ch_pool_deinit((ch_pool_t*)mf);
    if(res_code) {
        write_log_to_file(Error,"mf_close: deinitialization of chunk pool failed!\n");
        return res_code;
    }
   // write_log_to_file(Info,"mf_close: finished!\n\n\n");
    logger_deinit();

    return 0;
}

off_t mf_file_size(mf_handle_t mf) {
   // write_log_to_file(Info,"mf_file_size: started!\n");
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
   // write_log_to_file(Info,"mf_file_size: finished!\n");
    return sb.st_size;
}


int mf_unmap(mf_handle_t mf, mf_mapmem_handle_t mapmem_handle) {

   // write_log_to_file(Info,"mf_unmap: started!\n");
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
   // write_log_to_file(Info,"mf_unmap: finished!\n");

    return 0;
}

void *mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle) {
   // write_log_to_file(Info,"mf_map: started!\n");
    int res_code = 0;
//added
    if((offset + size > mf_file_size(mf)) && (offset < mf_file_size(mf))) {
        size = mf_file_size(mf) - offset;
        if(size%sysconf(_SC_PAGE_SIZE) != 0) {
        }
    }
//added
    if((mf == MF_OPEN_FAILED) || (mapmem_handle == NULL) || (offset > mf_file_size(mf)) || (offset < 0)) {
        *mapmem_handle = MF_OPEN_FAILED;
        write_log_to_file(Error,"mf_map: invalid input!\n");
        return NULL;
    }
    if(size == 0) {
        write_log_to_file(Error,"mf_map: size of mapping = 0!\n");
        return NULL;
    }
    off_t index = offset/get_chunk_size(1);
   // write_log_to_file(Debug,"mf_map: index is calculated!\n");
    off_t length = 0;

    if((offset + size)%get_chunk_size(1) != 0) {
       // write_log_to_file(Debug,"mf_map: (offset + size)%get_chunk_size(1) != 0\n");
        length = (offset + size)/get_chunk_size(1) - index + 1;
    } else {
        //write_log_to_file(Debug,"mf_map: (offset + size)%get_chunk_size(1) = 0\n");
        length = (offset + size)/get_chunk_size(1) - index;
    }
    ch_pool_t* ch_pool = (ch_pool_t *)mf;
    chunk_t** chunk = (chunk_t**)mapmem_handle;

    //*chunk = find_in_range_new(ch_pool -> h_table, offset, size);
    *chunk = take_value_ptr(ch_pool -> h_table, index, length);
    //*chunk = find_in_range(ch_pool -> h_table, offset, size);
    if ((*chunk) == NULL) {
        //write_log_to_file(Debug,"mf_map: t is no chunk, which contains needed range of bytes. The next step - init of such chunk!\n");
        int res_code = ch_init(index, length, ch_pool);
        if(res_code) {
            write_log_to_file(Error,"mf_map: initialization of chunk failed!\n");
            *mapmem_handle = MF_MAP_FAILED;
            return NULL; 
        }
        *chunk = take_value_ptr(ch_pool -> h_table, index, length);
    }
    void* ptr = NULL;
    ptr = ((*chunk) -> data) + offset - ((*chunk) -> index)*get_chunk_size(1);
    if(ptr == MF_MAP_FAILED) {
        write_log_to_file(Error,"mf_map: pointer to memory is NULL!\n");
        return MF_MAP_FAILED;
    }
   // write_log_to_file(Info,"mf_map: finished!\n");
    return ptr;
}


ssize_t mf_read(mf_handle_t mf, void* buf, size_t count, off_t offset) {
   // write_log_to_file(Info,"mf_read: started!\n");
    int res_code = 0;
    off_t size_of_file = mf_file_size(mf);
    if(size_of_file == 0) {
        write_log_to_file(Error,"mf_read: size of file = 0!\n");
        return 0;
    }

    if(((offset + (off_t)count) > size_of_file) && (offset < size_of_file)) {
        count = size_of_file - offset;
    }


    if((buf == NULL) || (offset < 0) || (mf == MF_OPEN_FAILED)) {
        write_log_to_file(Error,"mf_read: invalid input!\n");
        return 0;
    }
    if(count == 0) {
       // write_log_to_file(Debug,"mf_read: check size of bytes which you want read!\n");
        return 0;
    }
    ch_pool_t* ch_pool = (ch_pool_t*)mf;
    int fd = ch_pool -> fd;
    if(fd < 0) {
        write_log_to_file(Error,"mf_read: invalid value of file descriptor!\n");
        return -1;
    }

    off_t index = offset/get_chunk_size(1);
  //  write_log_to_file(Debug,"mf_read: index is calculated!\n");
    off_t length = 0;

    if((offset + count)%get_chunk_size(1) != 0) {
       // write_log_to_file(Debug,"mf_read: (offset + size)%get_chunk_size(1) != 0\n");
        length = (offset + count)/get_chunk_size(1) - index + 1;
    } else {
       // write_log_to_file(Debug,"mf_read: (offset + size)%get_chunk_size(1) = 0\n");
        length = (offset+ count)/get_chunk_size(1) - index;
    }

    //chunk_t* chunk_ptr = find_in_range_new(ch_pool -> h_table, offset, count);
    chunk_t* chunk_ptr = take_value_ptr(ch_pool -> h_table, index, length);
    //chunk_t* chunk_ptr = find_in_range(ch_pool -> h_table, offset, count);
    if(chunk_ptr == NULL) {
       // write_log_to_file(Debug,"mf_read: there is no chunk, which contains needed range of bytes. The next step - init of such chunk!\n");
        res_code = ch_init(index, length, ch_pool);
        if(res_code) {
            printf("%d\n",res_code);
            write_log_to_file(Error,"mf_read: initialization of chunk failed!\n");
            return res_code; 
        }
        chunk_ptr = take_value_ptr(ch_pool -> h_table, index, length);
    }
    buf = memcpy(buf,(const void*)((chunk_ptr -> data) + offset - (chunk_ptr -> index) * get_chunk_size(1)), count);
    // write_log_to_file(Info,"mf_read: finished!\n");
    return count;
}


ssize_t mf_write(mf_handle_t mf, const void* buf, size_t count, off_t offset) {
   // write_log_to_file(Info,"mf_write: started!\n");
    int res_code = 0;
    off_t size_of_file = mf_file_size(mf);
    if(size_of_file == 0) {
        write_log_to_file(Error,"mf_write: size of file = 0!\n");
        return 0;
    }

    if((offset + count > size_of_file) && (offset < size_of_file)) {
        count = size_of_file - offset;
    }

    if((buf == NULL) || (offset < 0) || (offset > size_of_file) || (mf == MF_OPEN_FAILED)) {
        write_log_to_file(Error,"mf_write: invalid input!\n");
        return -1;
    }

    ch_pool_t* ch_pool = (ch_pool_t*)mf;

    off_t index = offset/get_chunk_size(1);
   // write_log_to_file(Debug,"mf_write: index is calculated!\n");
    off_t length = 0;

    if((offset + count)%get_chunk_size(1) != 0) {
       // write_log_to_file(Debug,"mf_write: (offset + size)%get_chunk_size(1) != 0\n");
        length = (offset + count)/get_chunk_size(1) - index + 1;
    } else {
       // write_log_to_file(Debug,"mf_write: (offset + size)%get_chunk_size(1) != 0\n");
        length = (offset+ count)/get_chunk_size(1) - index;
    }

    //chunk_t* chunk_ptr = find_in_range_new(ch_pool -> h_table, offset, count);
    chunk_t* chunk_ptr = take_value_ptr(ch_pool -> h_table, index, length);
    //chunk_t* chunk_ptr = find_in_range(ch_pool -> h_table, offset, count);
    if(chunk_ptr == NULL) {
       // write_log_to_file(Debug,"mf_write: there is no chunk, which contains needed range of bytes. The next step - init of such chunk!\n");
        res_code = ch_init(index, length, ch_pool);
        if(res_code) {
            write_log_to_file(Error,"mf_read: initialization of chunk failed!\n");
            return res_code;
        }
        chunk_ptr = take_value_ptr(ch_pool -> h_table, index, length);
    }
    memcpy((void*)((char*)(chunk_ptr -> data) + offset - (chunk_ptr -> index) * get_chunk_size(1)), buf, count);
    return count;
}

