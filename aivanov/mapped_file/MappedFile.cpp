#include "MappedFile.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits>
#include <cstring>
#include <errno.h>

CMappedFile::CMappedFile(const char* fileName) :
	desc_(-1),
	root_(0, 0),
	entireFile_(NULL),
	cache_(NULL),
	size_(0)
{
	desc_ = open(fileName, O_RDWR | O_CREAT, 0755);
	
	if (isValid())
	{
		struct stat buf = {};
		fstat(desc_, &buf);
		size_ = buf.st_size;
		
		root_ = CFileRegion(0, size_);
		
		if (size_ && size_ <= MAX_ENTIRELY_MAPPED_SIZE)
			entireFile_ = map(0, size_, NULL);
	}
}

CMappedFile::~CMappedFile()
{
	if (entireFile_)
		entireFile_->removeReference();
		
	if (cache_)
		cache_->removeReference();
		
	close(desc_);
}

CFileRegion* CMappedFile::map(off_t offset, off_t size, void** address)
{
	long pageSize = sysconf(_SC_PAGE_SIZE);
	off_t mapOffset = (offset / pageSize) * pageSize;
	size_t memoryRegionSize = ((size + (offset - mapOffset) + pageSize - 1) / pageSize) * pageSize;
	size_t mapSize = std::min(memoryRegionSize, size_t(size_ - mapOffset));
	
	if (!mapSize)
	{
		errno = EINVAL;
		return NULL;
	}
	
	CFileRegion* newRegion = new CFileRegion(mapOffset, mapSize);
	CFileRegion* region = root_.takeChild(newRegion);
	
	if (region == newRegion)
		region->map(desc_);
	else
		delete newRegion;
		
	region->addReference();
	
	if (address)
		*address = region->getAddress(offset);
		
	return region;
}

ssize_t CMappedFile::fileCopy_(off_t offset, size_t size, uint8_t* to, const uint8_t* from)
{
	assert(!to || !from);
	assert(to || from);
	
	size = std::min(size, size_t(std::max(size_ - offset, off_t(0))));
	
	size_t bytesProcessed = 0;
	while (size)
	{
		CFileRegion* region = root_.maxAt(offset);
				
		if (!region)
		{
			if (cache_)
			{
				cache_->removeReference();
				
				if (!cache_->isReferenced())
					delete cache_;
			}
			
			region = cache_ = map(offset, CACHE_SIZE_PAGES * sysconf(_SC_PAGE_SIZE), NULL);
		}
		
		assert(region);
		
		void* mappedAddress = region->getAddress(offset);
		size_t mappedSize = region->getSizeAfter(offset);
		mappedSize = std::min(mappedSize, size);
		
		if (to)
			memcpy(to + bytesProcessed, mappedAddress, mappedSize);
		else
			memcpy(mappedAddress, from + bytesProcessed, mappedSize);
		
		bytesProcessed += mappedSize;
		offset += mappedSize;
		size -= mappedSize;
	}
	
	return bytesProcessed;
}

ssize_t CMappedFile::read(off_t offset, size_t size, uint8_t* buf)
{
	return fileCopy_(offset, size, buf, NULL);
}

ssize_t CMappedFile::write(off_t offset, size_t size, const uint8_t* buf)
{
	return fileCopy_(offset, size, NULL, buf);
}

bool CMappedFile::isValid()
{
	return desc_ >= 0;
}

off_t CMappedFile::getSize()
{
	return size_;
}

