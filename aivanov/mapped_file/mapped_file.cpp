#include "MappedFile.h"
#include <mapped_file.h>
//#define _LOG_ENABLED
#define _LOG_FLUSH true
#include "log/log.h"
#include <string.h>

//#define LOGE(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
//#define LOGI(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

mf_handle_t mf_open(const char *pathname)
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

ssize_t mf_read(mf_handle_t mf, void *buf, size_t size, off_t offset)
{
	LOGI("Reading %lu bytes at offset %ld from file %p to address %p)", size, offset, mf, buf);
	ssize_t bytes = ((CMappedFile*) mf)->read(offset, size, (uint8_t*) buf);
	
	if (bytes < 0)
		LOGE("Reading failed(%d): \"%s\"", errno, strerror(errno));
	else
		LOGI("Read %ld bytes", bytes);
		
	return bytes;	
}

ssize_t mf_write(mf_handle_t mf, const void *buf, size_t size, off_t offset)
{
	LOGI("Writing %lu bytes from address %p to file %p at offset %ld)", size, buf, mf, offset);
	ssize_t bytes = ((CMappedFile*) mf)->write(offset, size, (const uint8_t*) buf);
	
	if (bytes < 0)
		LOGE("Writing failed(%d): \"%s\"", errno, strerror(errno));
	else
		LOGI("Written %ld bytes", bytes);
		
	return bytes;
}

void* mf_map(mf_handle_t mf, off_t offset, size_t size, mf_mapmem_handle_t *mapmem_handle)
{
	LOGI("Mapping region of file %p of %lu bytes at offset %ld", mf, size, offset);
		
	void* data = NULL;
	*mapmem_handle = ((CMappedFile*) mf)->map(offset, size, &data);
	
	if (!*mapmem_handle)
	{
		LOGE("Mapping failed(%d): \"%s\"", errno, strerror(errno));
		return NULL;
	}
	
	
	LOGI("Mapped to address %p with handle %p", data, *mapmem_handle);
	
	return data;
}

int mf_unmap(mf_handle_t, mf_mapmem_handle_t mm)
{
	LOGI("Unmapping %p", mm);
	CFileRegion* region = ((CFileRegion*) mm);
	
	region->removeReference();
	
	if (!region->isReferenced())
		delete region;
		
	LOGI("Unmapped region");
	return 0;
}

off_t mf_file_size(mf_handle_t mf)
{
	LOGI("Getting file size of %p", mf);
	ssize_t size = ((CMappedFile*) mf)->getSize();
	LOGI("Size is %lu", size);
	return size;
}

