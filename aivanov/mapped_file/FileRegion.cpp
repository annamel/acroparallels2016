#include "FileRegion.h"
#include <sys/mman.h>
#include "MappedFile.h"


CFileRegion::CFileRegion(off_t offset, size_t size) :
	offset_(offset),
	size_(size),
	references_(0),
	children_(isOffsetLess_),
	parent_(NULL),
	address_(NULL)
{
}

CFileRegion* CFileRegion::takeChild(CFileRegion* region)
{
	assert(doesInclude(region));
	assert(!isReferenced() || isMapped());
	//if (isMapped())
	//	return this;
	
	auto next = children_.upper_bound(region);
		
	if (next != children_.begin())
	{
		auto prev = std::prev(next);
	
		if ((*prev)->doesInclude(region))
			return (*prev)->takeChild(region);
			
		if (isMapped() && !region->isReferenced())
			return this;
		
		if (region->doesInclude(*prev))
			region->readopt_(*prev);
	}
	else if (isMapped() && !region->isReferenced())
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
		
	assert((*prev)->isMapped());
	return (*prev)->doesInclude(&temp) ? *prev : NULL;
}

void CFileRegion::map(int fd)
{	
	#ifdef REGION_PROTECTION
	
		uint8_t* address = (uint8_t*) mmap(NULL, memoryRegionSize + 2 * CMappedFile::pageSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
		if (address == MAP_FAILED)
		{
			address_ = NULL;
			return;
		}
	
		address_ = (uint8_t*) mmap(address + CMappedFile::pageSize, size_, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, region->offset_);
	#else
		address_ = (uint8_t*) mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset_);
	#endif

	if (address_ == MAP_FAILED)
		address_ = NULL;
}

void CFileRegion::unmap_()
{
	if (address_)
	{
		#ifdef REGION_PROTECTION
			munmap(address_ - CMappedFile::pageSize, ((size_ + CMappedFile::pageSize - 1) / CMappedFile::pageSize + 2) * CMappedFile::pageSize);
		#else
			
			munmap(address_, size_);
		#endif
	}
}

void CFileRegion::adopt_(CFileRegion* child)
{
	assert(!child->parent_);
	
	child->iteratorInParent_ = children_.insert(child).first;
	child->parent_ = this;
}

void CFileRegion::orphan_()
{
	assert(parent_);
	
	parent_->children_.erase(iteratorInParent_);
	parent_ = NULL;
}

void CFileRegion::readopt_(CFileRegion* child)
{
	child->orphan_();
	adopt_(child);
}

CFileRegion::~CFileRegion()
{	
	//assert(!isReferenced());
	
	for (auto it = children_.begin(); it != children_.end(); it = children_.begin())
	{
		CFileRegion* child = *it;
		
		if (parent_)
			parent_->takeChild(child);
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

bool CFileRegion::isOffsetLess_(CFileRegion* a, CFileRegion* b)
{
	return a->offset_ < b->offset_;
}

bool CFileRegion::doesInclude(off_t offset)
{
	return offset >= offset_ && offset < offset_ + off_t(size_);
}

bool CFileRegion::isMapped()
{
	return !!address_;
}

CFileRegion* CFileRegion::getParent()
{
	return parent_;
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

size_t CFileRegion::getSize()
{
	return size_;
}


