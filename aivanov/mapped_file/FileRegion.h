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
	
	off_t									offset_;
	off_t									size_;
	int										references_;
	std::set<CFileRegion*, TCompare>		children_;
	CFileRegion*							parent_;
	uint8_t*								address_;

	static bool isLess_(CFileRegion* a, CFileRegion* b);
	
public:
	CFileRegion(off_t offset, off_t size);
	~CFileRegion();
	
	bool operator <(const CFileRegion& a);
	bool operator !();
	
	void adopt(CFileRegion* child);
	void readopt(CFileRegion* child);
	void orphan();
	void addReference();
	void removeReference();
	bool doesInclude(const CFileRegion* a);
	void unmap();
	CFileRegion* takeChild(CFileRegion* region);
	CFileRegion* maxAt(off_t offset);
	
	friend class CMappedFile;
};

#endif

