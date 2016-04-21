#include "MappedFile.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits>

CMappedFile::CMappedFile(const char* fileName) :
	desc_(-1),
	root_(0, std::max(2000l, std::numeric_limits<off_t>::max()))
{
	desc_ = open(fileName, O_RDWR);
}

CMappedFile::~CMappedFile()
{
	close(desc_);
}

CFileRegion* CMappedFile::map(off_t offset, off_t size, void** address)
{
	long pageSize = sysconf(_SC_PAGE_SIZE);
	off_t properOffset = (offset / pageSize) * pageSize;
	size_t properSize = size + (offset - properOffset);

	CFileRegion* newRegion = new CFileRegion(properOffset, properSize);
	CFileRegion* region = root_.takeChild(newRegion);
	region->addReference();
	
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
	
	*address = region->address_ + (offset - region->offset_);
	return region;
}

ssize_t CMappedFile::read(off_t offset, size_t size, void *buf)
{
	lseek(desc_, offset, SEEK_SET);
	ssize_t status = ::read(desc_, buf, size);
	return status;
}

ssize_t CMappedFile::write(off_t offset, size_t size, const void *buf)
{
	lseek(desc_, offset, SEEK_SET);
	ssize_t status = ::write(desc_, buf, size);
	return status;
}

bool CMappedFile::isValid()
{
	return desc_ >= 0;
}

off_t CMappedFile::getSize()
{
	struct stat buf;
	fstat(desc_, &buf);
	return buf.st_size;
}

