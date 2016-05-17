#include "MappedFile.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits>
#include <cstring>
#include <errno.h>

size_t CMappedFile::pageSize = 0;

#ifdef MAPPED_FILE_MT
	#define MF_LOCK_READ() CHECK(!pthread_rwlock_rdlock(&rwLock_))
	#define MF_LOCK_WRITE() CHECK(!pthread_rwlock_wrlock(&rwLock_))
	#define MF_UNLOCK() CHECK(!pthread_rwlock_unlock(&rwLock_))
#else
	#define MF_LOCK_READ()
	#define MF_LOCK_WRITE()
	#define MF_UNLOCK()
#endif

#define REGION_RECT_UNIT (pageSize * 0x10000)

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
	
	#ifdef MAPPED_FILE_MT
		CHECK(!pthread_rwlock_init(&rwLock_, NULL));
	#endif

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
	MF_LOCK_WRITE();
	MF_LOCK_READ();
	
	if (entireFile_)
		unmap(entireFile_);
	
	if (cache_)
		unmap(cache_);
	
	#ifdef MAPPED_FILE_MT
		CHECK(!pthread_rwlock_destroy(&rwLock_));
	#endif
		
	close(desc_);
}

CFileRegion* CMappedFile::map(off_t offset, off_t size, void** address)
{
	MF_LOCK_WRITE();
	
	if (offset + size < size_)
	{
		CFileRegion* biggerRegion = map(offset, size_ - offset, address);
		
		if (biggerRegion)
			return biggerRegion;
	}
	
	off_t mapOffset = (offset / REGION_RECT_UNIT) * REGION_RECT_UNIT;
	size_t memoryRegionSize = ((size + (offset - mapOffset) + REGION_RECT_UNIT - 1) / REGION_RECT_UNIT) * REGION_RECT_UNIT;
	size_t mapSize = std::min(memoryRegionSize, size_t(size_ - mapOffset));
	
	if (!mapSize)
	{
		errno = EINVAL;
		return NULL;
	}
	
	CFileRegion* newRegion = new CFileRegion(mapOffset, mapSize);
	
	//MF_LOCK_WRITE()
	CFileRegion* region = root_.takeChild(newRegion);
	//MF_UNLOCK()
	
	if (region == newRegion)
	{
		region->map(desc_);
		
		if (!region->isMapped())
		{
			//MF_LOCK_WRITE()
			while (!region->isMapped() && shrinkCache_())
				region->map(desc_);
			//MF_UNLOCK()
		}
		
		if (!region->isMapped())
		{
			delete region;
			MF_UNLOCK();
			return NULL;
		}
	}
	else
	{
		//MF_LOCK_WRITE()
		if (!region->isReferenced())
			regionPool_.erase(region->poolIterator);
		//MF_UNLOCK()
			
		delete newRegion;
	}
		
	region->addReference();
	
	if (address)
		*address = region->getAddress(offset);
	
	MF_UNLOCK();
	return region;
}

ssize_t CMappedFile::fileCopy_(off_t offset, size_t size, uint8_t* to, const uint8_t* from)
{
	assert(!to || !from);
	assert(to || from);
	
	size = std::min(size, size_t(std::max(size_ - offset, off_t(0))));
	
	MF_LOCK_READ();
		
	size_t bytesProcessed = 0;
	while (size)
	{
		CFileRegion* region = root_.maxAt(offset);
				
		if (!region)
		{
			MF_UNLOCK();
			
			if (cache_)
				unmap(cache_);
				
			region = cache_= map(offset, 1, NULL);
			
			MF_LOCK_READ();
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
	
	MF_UNLOCK();
	
	return bytesProcessed;
}

void CMappedFile::unmap(CFileRegion* region)
{
	MF_LOCK_WRITE();
	
	assert(region);
	assert(region->isMapped());
	assert(region->getParent());
	
	region->removeReference();

	if (!region->isReferenced())
	{
		if (!region->getParent()->isMapped())
			region->poolIterator = regionPool_.insert(regionPool_.end(), region);
		else
			delete region;
	}
	
	MF_UNLOCK();
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

