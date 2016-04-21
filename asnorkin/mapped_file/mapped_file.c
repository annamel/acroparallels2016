#include "mapped_file.h"




mf_handle_t mf_open(const char *pathname, size_t max_memory_usage)
{
    if(!pathname)
        return EINVAL;

    int flags = O_CREAT | O_RDWR;
    int perms = 0666;
    int fd = open(pathname, flags, perms);
    if(fd == -1)
        return MF_OPEN_FAILED;

    chpool_t *chpool = NULL;

    int prot = (flags & O_RDWR) ? PROT_READ | PROT_WRITE :
               (flags & O_RDONLY) ? PROT_READ : PROT_WRITE;
    int error = chpool_construct(max_memory_usage, fd, prot, chpool);
    if(error)
        return MF_OPEN_FAILED;

    mf_handle_t mf = (void *)chpool;

    return mf;
}



int mf_close(mf_handle_t mf)
{
    if(!mf)
        return -1;

    if(chpool_destruct((chpool_t *)&mf) != 0)
        return -1;
    else
        return 0;
}



ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void *buf)
{
    int read_bytes_cnt = 0;
    chpool_t *chpool = (chpool_t *)mf;
    chunk_t *chunk = NULL;

    int error = chunk_find(chunk, chpool, size, offset);
    void *tmp_buf = NULL;
    if(error == 0)
    {
        error = chunk_get_mem(chunk, offset, tmp_buf);
        if(error)
            return -1;

        memcpy(buf, tmp_buf, size);
        read_bytes_cnt = size;
    }
    else if(error == ENOKEY)
    {
        read_bytes_cnt = pread(chpool_fd(chpool), buf, size, offset);
        if(read_bytes_cnt == -1)
            return -1;
    }
    else
        return -1;

    return read_bytes_cnt;
}



ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void *buf)
{
    int write_bytes_cnt = 0;
    chpool_t *chpool = (chpool_t *)mf;
    chunk_t *chunk = NULL;

    int error = chunk_find(chpool, offset, size, chunk);
    void *tmp_buf = NULL;
    if(error == 0)
    {
        error = chunk_get_mem(chunk, offset, tmp_buf);
        if(error)
            return -1;

        memcpy(buf, tmp_buf, size);
        write_bytes_cnt = size;
    }
    else if(error == ENOKEY)
    {
        write_bytes_cnt = pwrite(chpool_fd(chpool), buf, size, offset);
        if(write_bytes_cnt == -1)
            return -1;
    }
    else
        return -1;

    return write_bytes_cnt;
}



mf_mapmem_t *mf_map(mf_handle_t mf, off_t offset, size_t size)
{
    mf_mapmem_t *mapmem = (mf_mapmem_t *)calloc(1, sizeof(mf_mapmem_t));
    if(!mapmem)
        return MF_MAP_FAILED;

    mapmem->handle = NULL;

    chpool_t *chpool = (chpool_t *)mf;
    chunk_t *chunk = (chunk_t *)mapmem->handle;

    if(chunk_acquire(chpool, offset, size, chunk))
        return MF_MAP_FAILED;

    if(chunk_get_mem(chunk, offset, mapmem->ptr))
        return MF_MAP_FAILED;
    else
        return mapmem;
}



int mf_unmap(mf_mapmem_t *mm)
{
    if(chunk_release((chunk_t *)mm->handle))
        return -1;

    free(mm);

    return 0;
}



ssize_t mf_file_size(mf_handle_t mf)
{
    off_t size = 0;
    int fd = chpool_fd((chpool_t *)mf);

    struct stat sb = {0};

    int error = fstat(fd, &sb);
    if(error == -1)
        return -1;

    return sb.st_size;
}
