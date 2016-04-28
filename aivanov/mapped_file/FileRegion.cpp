#include "FileRegion.h"
#include <sys/mman.h>


CFileRegion::CFileRegion(off_t offset, size_t size) :
	offset_(offset),
	size_(size),
	references_(0),
	children_(isLess_),
	parent_(NULL),
	address_(NULL)
{
}

CFileRegion* CFileRegion::takeChild(CFileRegion* region)
{
	assert(doesInclude(region));
	
	auto next = children_.upper_bound(region);
		
	if (next != children_.begin())
	{
		auto prev = std::prev(next);
	
		if ((*prev)->doesInclude(region))
			return (*prev)->takeChild(region);
			
		if (parent_ && !region->isReferenced())
			return this;
		
		if (region->doesInclude(*prev))
			region->readopt_(*prev);
	}
	else if (parent_ && !region->isReferenced())
		return this;
	
	while (next != children_.end() && region->doesInclude(*next))
		region->readopt_(*next++);
	
	adopt_(region);
	return region;
}

CFileRegion* CFileRegion::maxAt(off_t offset)
{
	CFileRegion temp(offset, 1);
	
	auto next = children_.upper_bound(&temp);
	if (next == children_.begin())
		return NULL;
		
	auto prev = std::prev(next);
	if (prev == children_.end())
		return NULL;
		
	assert((*prev)->isReferenced());
	return (*prev)->doesInclude(&temp) ? *prev : NULL;
}

//#define REGION_PROTECTION
void CFileRegion::map(int fd)
{	
	#ifdef REGION_PROTECTION
		long pageSize = sysconf(_SC_PAGE_SIZE);
	
		uint8_t* address = (uint8_t*) mmap(NULL, memoryRegionSize + 2 * pageSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
		assert(address != MAP_FAILED);
	
		address_ = (uint8_t*) mmap(address + pageSize, size_, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, region->offset_);
	#else
		address_ = (uint8_t*) mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset_);
	#endif

	assert(address_ != MAP_FAILED);
}

void CFileRegion::unmap_()
{
	if (address_)
	{
		#ifdef REGION_PROTECTION
			long pageSize = sysconf(_SC_PAGE_SIZE);
			munmap(address_ - pageSize, ((size_ + pageSize - 1) / pageSize + 2) * pageSize);
		#else
			
			munmap(address_, size_);
		#endif
	}
}

void CFileRegion::adopt_(CFileRegion* child)
{
	assert(!child->parent_);
	
	children_.insert(child);
	child->parent_ = this;
}

void CFileRegion::orphan_()
{
	assert(parent_);
	
	parent_->children_.erase(this);
	parent_ = NULL;
}

void CFileRegion::readopt_(CFileRegion* child)
{
	child->orphan_();
	adopt_(child);
}

CFileRegion::~CFileRegion()
{	
	assert(!isReferenced());
	
	for (auto it = children_.begin(); it != children_.end(); it = children_.begin())
	{
		CFileRegion* child = *it;
		
		if (parent_)
			parent_->readopt_(child);
		else
		{
			child->orphan_();
			delete child;
		}
	}
	
	if (parent_)
		orphan_();
		
	unmap_();
}

bool CFileRegion::operator <(const CFileRegion& a)
{
	return offset_ < a.offset_;		
}

bool CFileRegion::doesInclude(const CFileRegion* a)
{
	return offset_ <= a->offset_ && offset_ + size_ >= a->offset_ + a->size_;
}

void CFileRegion::addReference()
{
	assert(address_);
	references_++;
}

void CFileRegion::removeReference()
{
	assert(address_);
	assert(references_);
	references_--;
}

bool CFileRegion::isReferenced()
{
	return !!references_;
}

bool CFileRegion::isLess_(CFileRegion* a, CFileRegion* b)
{
	return *a < *b;
}

bool CFileRegion::doesInclude(off_t offset)
{
	return offset >= offset_ && offset < offset_ + off_t(size_);
}

void* CFileRegion::getAddress(off_t offset)
{
	assert(address_);
	assert(doesInclude(offset));
	
	return address_ + (offset - offset_);
}

size_t CFileRegion::getSizeAfter(off_t offset)
{
	assert(doesInclude(offset));
	
	return size_ - (offset - offset_);
}



