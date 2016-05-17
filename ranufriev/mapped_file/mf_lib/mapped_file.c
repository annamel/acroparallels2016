#include "./mapped_file.h"
#include "../hash_table/hash_table.h"

#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include <malloc.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#include <sys/mman.h>



#define HASH_TABLE_SIZE 4096

//==============================================================================
// Declaring of structures
//------------------------------------------------------------------------------
typedef struct Mapped_file
        {
        int fd;

        int is_initialized;
        int is_fully_mapped; // -1  -  mmap was unsuccessful
                             //  0  -  initial value
                             //  1  -  file was entirely mmaped
        void* mmaped_ptr;

        off_t file_size;
        long page_size;

        hash_table_ptr ht;
        } mf_handle_str;

typedef mf_handle_str* mf_handle_ptr;
//------------------------------------------------------------------------------
typedef struct Mapped_memory
        {
        mf_handle_ptr mf;
        off_t offset;
        size_t size;

        size_t ref_cnt;
        } mf_mapmem_handle_str;

typedef mf_mapmem_handle_str* mf_mapmem_handle_ptr;
//==============================================================================


//==============================================================================
// Prototypes of internal functions
//------------------------------------------------------------------------------
int _lazy_init (mf_handle_ptr mf);

size_t hashing_func (const hash_key_t* hash_key, size_t hash_key_size, size_t hash_table_size);
//==============================================================================
// Realisation of functions
//------------------------------------------------------------------------------

/*
 * Returns NULL on failure.
 */
mf_handle_t mf_open (const char *pathname)
{
int fd = 0;
off_t file_size = 0;
long page_size = 0;
mf_handle_ptr mf = NULL;

if ((fd = open (pathname, O_RDWR)) == -1)
        return MF_OPEN_FAILED; // 'errno' was set by 'calloc' appropriately

if ((file_size = lseek (fd, 0, SEEK_END)) == (off_t)(-1))
        return MF_OPEN_FAILED;

if ((page_size = sysconf(_SC_PAGESIZE)) == -1)
        return MF_OPEN_FAILED;

if ((mf = (mf_handle_ptr)calloc (1, sizeof (mf_handle_str))) == NULL)
        return MF_OPEN_FAILED;

mf->fd = fd;

mf->is_initialized  = 0;
mf->is_fully_mapped = 0;
mf->mmaped_ptr      = NULL;

mf->file_size = file_size;
mf->page_size = page_size;

mf->ht = NULL;

return mf;
}

int _lazy_init (mf_handle_ptr mf)
{
mf->mmaped_ptr = mmap (NULL, mf->file_size, PROT_READ | PROT_WRITE, MAP_SHARED, mf->fd, 0);
if (mf->mmaped_ptr == MAP_FAILED)
        {
        mf->is_fully_mapped = -1;
        mf->mmaped_ptr = NULL;

        // if whole file was mapped, I do not need any hash_table
        if (Hash_table_construct (&(mf->ht), HASH_TABLE_SIZE, hashing_func) == -1)
                return -1;
        }
else
        {
        mf->is_fully_mapped = 1;
        mf->ht = NULL;
        }

mf->is_initialized = 1;

return 0;
}

size_t hashing_func (const hash_key_t* hash_key, size_t hash_key_size, size_t hash_table_size)
{
off_t a = *(off_t*)hash_key;

a = (a^0xdeadbeef) + (a<<4);
a = a ^ (a>>10);
a = a + (a<<7);
a = a ^ (a>>13);

return llabs(a) % hash_table_size;
}

/*
 * Returns 0 on success and -1 on failure
 */
int mf_close (mf_handle_t mf)
{
mf_handle_ptr mf_ptr = (mf_handle_ptr)mf;

if (mf == NULL)
        {
        errno = EINVAL;
        return -1;
        }

if (mf_ptr->is_initialized == 1)
        {
        if (mf_ptr->is_fully_mapped == 1)
                {
                if (munmap (mf_ptr->mmaped_ptr, mf_ptr->file_size) != 0)
                        return -1;
                }
        else
                {
                if (Hash_table_destruct (&(mf_ptr->ht)) != 0)
                        return -1;
                }
        }

if (close (mf_ptr->fd) != 0)
        return -1;

free (mf);

return 0;
}

/*
 * Returns NULL on failure
 */
void *mf_map (mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle)
{
mf_handle_ptr mf_ptr = (mf_handle_ptr)mf;
// mf_mapmem_handle_ptr mapmem_ptr = (mf_mapmem_handle_ptr)mapmem_handle;

if ((mf == NULL) || (offset < 0) || (offset > mf_ptr->file_size) || (size <= 0) || (mapmem_handle == NULL))
        {
        errno = EINVAL;
        return MF_MAP_FAILED;
        }

if ((offset + size) > mf_ptr->file_size)
        size = mf_ptr->file_size - offset;

if (mf_ptr->is_initialized != 1)
        assert (_lazy_init (mf_ptr) == 0);

if (mf_ptr->is_fully_mapped == 1)
        return mf_ptr->mmaped_ptr + offset;

/*
mapmem_ptr->mf     = mf_ptr;
mapmem_ptr->offset = offset;
mapmem_ptr->size   = size;
*/

return MF_MAP_FAILED;
}

/*
 * Returns 0 on success and -1 on failure
 */
int mf_unmap (mf_handle_t mf, mf_mapmem_handle_t mapmem_handle)
{
mf_handle_ptr mf_ptr = (mf_handle_ptr)mf;

// if ((mf == NULL) || (mapmem_handle == NULL))
if (mf == NULL)
        {
        errno = EINVAL;
        return -1;
        }

if (mf_ptr->is_fully_mapped == 1)
        return 0;




return 0;
}

/*
 * Returns -1 on failure
 */
ssize_t mf_read (mf_handle_t mf, void* buf, size_t count, off_t offset)
{
mf_handle_ptr mf_ptr = (mf_handle_ptr)mf;

if ((mf == NULL) || (buf == NULL) || (count <= 0) || (offset < 0) || (offset > mf_ptr->file_size))
        {
        errno = EINVAL;
        return -1;
        }

if ((offset + count) > mf_ptr->file_size)
        count = mf_ptr->file_size - offset;

if (mf_ptr->is_initialized != 1)
        assert (_lazy_init (mf_ptr) == 0);

if (mf_ptr->is_fully_mapped == 1)
        {
        if (!memcpy (buf, mf_ptr->mmaped_ptr + offset, count))
                return -1;

        return count;
        }




return -1;
}

/*
 * Returns -1 on failure
 */
ssize_t mf_write (mf_handle_t mf, const void* buf, size_t count, off_t offset)
{
mf_handle_ptr mf_ptr = (mf_handle_ptr)mf;

if ((mf == NULL) || (buf == NULL) || (count <= 0) || (offset < 0) || (offset > mf_ptr->file_size))
        {
        errno = EINVAL;
        return -1;
        }

if ((offset + count) > mf_ptr->file_size)
        count = mf_ptr->file_size - offset;

if (mf_ptr->is_initialized != 1)
        assert (_lazy_init (mf_ptr) == 0);

if (mf_ptr->is_fully_mapped == 1)
        {
        if (!memcpy (mf_ptr->mmaped_ptr + offset, buf, count))
                return -1;

        return count;
        }




return -1;
}

/*
 * Returns -1 on failure
 */
off_t mf_file_size (mf_handle_t mf)
{
mf_handle_ptr mf_ptr = (mf_handle_ptr)mf;

if (mf == NULL)
       {
       errno = EINVAL;
       return -1;
       }

return mf_ptr->file_size;
}

//==============================================================================




