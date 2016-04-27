#include "FileRegion.h"
#include <sys/mman.h>


CFileRegion::CFileRegion(off_t offset, off_t size) :
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
			
		if (parent_ && !*region)
			return this;
		
		if (region->doesInclude(*prev))
			region->readopt(*prev);
	}
	else if (parent_ && !*region)
		return this;
	
	while (next != children_.end() && region->doesInclude(*next))
		region->readopt(*next++);
	
	adopt(region);
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
		
	assert(!!**prev);
	return (*prev)->doesInclude(&temp) ? *prev : NULL;
}

void CFileRegion::unmap()
{
	removeReference();
}

void CFileRegion::adopt(CFileRegion* child)
{
	assert(!child->parent_);
	
	children_.insert(child);
	child->parent_ = this;
}

void CFileRegion::orphan()
{
	assert(parent_);
	
	parent_->children_.erase(this);
	parent_ = NULL;
}

void CFileRegion::readopt(CFileRegion* child)
{
	child->orphan();
	adopt(child);
}

CFileRegion::~CFileRegion()
{	
	assert(!*this);
	
	for (auto it = children_.begin(); it != children_.end(); it = children_.begin())
	{
		CFileRegion* child = *it;
		
		if (parent_)
			parent_->readopt(child);
		else
		{
			child->orphan();
			delete child;
		}
	}
	
	if (parent_)
		orphan();
	
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
	references_++;
}

void CFileRegion::removeReference()
{
	assert(references_);
	references_--;
}

bool CFileRegion::operator !()
{
	return !references_;
}

bool CFileRegion::isLess_(CFileRegion* a, CFileRegion* b)
{
	return *a < *b;
}

