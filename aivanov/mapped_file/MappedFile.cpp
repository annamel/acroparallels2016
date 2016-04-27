#include "MappedFile.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits>
#include <cstring>

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
		root_.size_ = size_;
		
		if (size_ && size_ <= MAX_ENTIRELY_MAPPED_SIZE)
		{
			entireFile_ = map(0, size_, NULL);
			assert(entireFile_);
		}
	}
}

CMappedFile::~CMappedFile()
{
	if (entireFile_)
		entireFile_->unmap();
		
	if (cache_)
		cache_->unmap();
		
	close(desc_);
}

CFileRegion* CMappedFile::map(off_t offset, off_t size, void** address)
{
	long pageSize = sysconf(_SC_PAGE_SIZE);
	off_t properOffset = (offset / pageSize) * pageSize;
	size_t properSize = ((size + (offset - properOffset) + pageSize - 1) / pageSize) * pageSize;
	properSize = std::min(properSize, size_t(size_ - properOffset));
	CFileRegion* newRegion = new CFileRegion(properOffset, properSize);
	CFileRegion* region = root_.takeChild(newRegion);
	assert(region->doesInclude(newRegion));
	region->addReference();
	assert(region->references_ > 0);
	
	if (region == newRegion)
	{
		region->address_ = (uint8_t*) mmap(NULL, region->size_, PROT_READ | PROT_WRITE, MAP_SHARED, desc_, region->offset_);
		
		if (!region->address_ || region->address_ == MAP_FAILED)
		{
			delete region;
			return NULL;
		}
	}
	else
		delete newRegion;
	
	if (address)
		*address = region->address_ + (offset - region->offset_);
		
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
				cache_->unmap();
				
				if (!*cache_)
					delete cache_;
			}
			
			region = cache_ = map(offset, CACHE_SIZE_PAGES * sysconf(_SC_PAGE_SIZE), NULL);
		}
		
		assert(region);
		
		off_t offsetAtRegion = offset - region->offset_;
		void* mappedAddress = region->address_ + offsetAtRegion;
		size_t mappedSize = region->size_ - offsetAtRegion;
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

