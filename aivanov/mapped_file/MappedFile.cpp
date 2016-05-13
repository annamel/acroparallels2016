#include "MappedFile.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits>
#include <cstring>
#include <errno.h>

size_t CMappedFile::pageSize = 0;


CMappedFile::CMappedFile(const char* fileName) :
	desc_(-1),
	root_(0, 0),
	entireFile_(NULL),
	size_(0),
	cache_(NULL)
{
	if (!pageSize)
		pageSize = sysconf(_SC_PAGE_SIZE);

	desc_ = open(fileName, O_RDWR | O_CREAT, 0755);
	
	if (isValid())
	{
		struct stat buf = {};
		fstat(desc_, &buf);
		size_ = buf.st_size;
		
		root_ = CFileRegion(0, size_);
		
		entireFile_ = map(0, size_, NULL);
	}
}

CMappedFile::~CMappedFile()
{
	if (entireFile_)
		unmap(entireFile_);
	
	if (cache_)
		unmap(cache_);
		
	close(desc_);
}

#define REGION_RECT_UNIT (pageSize * 0x10000)

CFileRegion* CMappedFile::map(off_t offset, off_t size, void** address)
{
	off_t mapOffset = (offset / REGION_RECT_UNIT) * REGION_RECT_UNIT;
	size_t memoryRegionSize = ((size + (offset - mapOffset) + REGION_RECT_UNIT - 1) / REGION_RECT_UNIT) * REGION_RECT_UNIT;
	size_t mapSize = std::min(memoryRegionSize, size_t(size_ - mapOffset));
	
	if (!mapSize)
	{
		errno = EINVAL;
		return NULL;
	}
	
	CFileRegion* newRegion = new CFileRegion(mapOffset, mapSize);
	CFileRegion* region = root_.takeChild(newRegion);
	
	if (region == newRegion)
	{
		do
		{
			region->map(desc_);
			
		} while (!region->isMapped() && shrinkCache_());
		
		if (!region->isMapped())
		{
			delete region;
			return NULL;
		}
	}
	else
	{
		if (!region->isReferenced())
			regionPool_.erase(region->poolIterator);
			
		delete newRegion;
	}
		
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
				unmap(cache_);
				
			region = cache_= map(offset, 1, NULL);
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

void CMappedFile::unmap(CFileRegion* region)
{
	assert(region);
	assert(region->isMapped());
	assert(region->getParent());
	
	region->removeReference();

	if (!region->isReferenced())
	{
		if (!region->getParent()->isMapped())
		{
			region->poolIterator = regionPool_.insert(regionPool_.end(), region);
		}
		else
		{
			printf("deleting %p\n", region);
			delete region;
		}
	}
}

bool CMappedFile::shrinkCache_()
{
	auto unmapped = regionPool_.begin();
	if (unmapped == regionPool_.end())
		return false;	
	
	delete *unmapped;
	regionPool_.erase(unmapped);
	return true;
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

