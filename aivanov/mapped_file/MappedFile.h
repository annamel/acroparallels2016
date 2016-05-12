#ifndef __MAPPEDFILE__
#define __MAPPEDFILE__

#include "FileRegion.h"
#include <queue>

#define CACHE_SIZE_PAGES 2
#define MAX_POOL_SIZE (1 << 30)

class CMappedFile
{

private:
	typedef std::function<bool (CFileRegion*, CFileRegion*)> TCompare;
	
	int									desc_;
	CFileRegion							root_;
	CFileRegion*						entireFile_;
	CFileRegion*						cache_;
	off_t								size_;
	std::set<CFileRegion*, TCompare>	regionPool_;
	size_t								poolSize_;
	
	static bool isUnmappedMoreLikely_(CFileRegion* a, CFileRegion* b);
	
	ssize_t fileCopy_(off_t offset, size_t size, uint8_t* to, const uint8_t* from);
	
public:
	static size_t						pageSize; //better place?

	CMappedFile(const char* fileName);
	~CMappedFile();

	CFileRegion* map(off_t offset, off_t size, void** address);
	void unmap(CFileRegion* region);
	off_t getSize();
	ssize_t read(off_t offset, size_t size, uint8_t* buf);
	ssize_t write(off_t offset, size_t size, const uint8_t* buf);
	bool isValid();
	
};

#endif


