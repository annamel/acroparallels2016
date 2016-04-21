#include "MappedFile.h"
#include "mapped_file.h"
//#define _LOG_ENABLED
#define _LOG_FLUSH true
#include "log/log.h"
#include <string.h>

mf_handle_t mf_open(const char *pathname, size_t)
{
	LOGI("Opening file %s", pathname);
	CMappedFile* mapped_file = new CMappedFile(pathname);
	
	if (!mapped_file || !mapped_file->isValid())
	{
		LOGE("Opening file failed");
		return NULL;
	}
		
	LOGI("Opened file with handle %p", mapped_file);
	return mapped_file;
}

int mf_close(mf_handle_t mf)
{
	LOGI("Closing file %p", mf);
	delete (CMappedFile*) mf;
	LOGI("Closed file");
	
	return 0;
}

ssize_t mf_read(mf_handle_t mf, off_t offset, size_t size, void *buf)
{
	LOGI("Reading %lu bytes at offset %ld from file %p to address %p)", size, offset, mf, buf);
	ssize_t bytes = ((CMappedFile*) mf)->read(offset, size, buf);
	
	if (bytes < 0)
		LOGE("Reading failed(%d): \"%s\"", errno, strerror(errno));
	else
		LOGI("Read %ld bytes", bytes);
		
	return bytes;	
}

ssize_t mf_write(mf_handle_t mf, off_t offset, size_t size, const void *buf)
{
	LOGI("Writing %lu bytes from address %p to file %p at offset %ld)", size, buf, mf, offset);
	ssize_t bytes = ((CMappedFile*) mf)->write(offset, size, buf);
	
	if (bytes < 0)
		LOGE("Writing failed(%d): \"%s\"", errno, strerror(errno));
	else
		LOGI("Written %ld bytes", bytes);
		
	return bytes;
}

mf_mapmem_t *mf_map(mf_handle_t mf, off_t offset, size_t size)
{
	LOGI("Mapping region of file %p of %lu bytes at offset %ld", mf, size, offset);
	
	struct mf_mapped_memory* mm = new mf_mapped_memory;
	if (!mm)
	{
		LOGE("Mapping failed(%d): \"%s\"", errno, strerror(errno));
		return NULL;
	}
		
	mm->handle = ((CMappedFile*) mf)->map(offset, size, &mm->ptr);
	
	if (!mm->handle)
	{
		delete mm;
		LOGE("Mapping failed(%d): \"%s\"", errno, strerror(errno));
		return NULL;
	}
	
	
	LOGI("Mapped to address %p with mf_mapmem_t* %p and handle %p", mm->ptr, mm, mm->handle);
	
	return mm;
}

int mf_unmap(mf_mapmem_t *mm)
{
	LOGI("Unmapping %p", mm);
	CFileRegion* region = ((CFileRegion*) mm->handle);
	
	region->unmap();
	
	if (!*region)
		delete region;
		
	delete mm;
		
	LOGI("Unmapped region");
	return 0;
}

ssize_t mf_file_size(mf_handle_t mf)
{
	LOGI("Getting file size of %p", mf);
	ssize_t size = ((CMappedFile*) mf)->getSize();
	LOGI("Size is %lu", size);
	return size;
}

