#ifndef __FILEREGION__
#define __FILEREGION__

#include <sys/types.h>
#include <cassert>
#include <set>
#include <functional>
#include <stdint.h>

class CFileRegion
{
private:
	typedef std::function<bool (CFileRegion*, CFileRegion*)> TCompare;
	
	off_t										offset_;
	size_t										size_;
	int											references_;
	std::set<CFileRegion*, TCompare>			children_;
	CFileRegion*								parent_;
	uint8_t*									address_;
	std::set<CFileRegion*, TCompare>::iterator	iteratorInParent_;

	static bool isOffsetLess_(CFileRegion* a, CFileRegion* b);
	
	void adopt_(CFileRegion* child);
	void readopt_(CFileRegion* child);
	void orphan_();
	void unmap_();
	
public:
	CFileRegion(off_t offset, size_t size);
	~CFileRegion();
	
	bool operator <(const CFileRegion& a);
	
	void addReference();
	void removeReference();
	bool isReferenced();
	bool isMapped();
	
	CFileRegion* takeChild(CFileRegion* region);
	CFileRegion* maxAt(off_t offset);
	bool doesInclude(const CFileRegion* a);
	bool doesInclude(off_t offset);
	void* getAddress(off_t offset);
	size_t getSizeAfter(off_t offset);
	size_t getSize();
	CFileRegion* getParent();
	
	void map(int fd);
};

#endif

