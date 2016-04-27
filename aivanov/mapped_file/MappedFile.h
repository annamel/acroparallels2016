#ifndef __MAPPEDFILE__
#define __MAPPEDFILE__

#include "FileRegion.h"

#define MAX_ENTIRELY_MAPPED_SIZE (1 << 27)
#define CACHE_SIZE_PAGES 2

class CMappedFile
{

private:
	int				desc_;
	CFileRegion		root_;
	CFileRegion*	entireFile_;
	CFileRegion*	cache_;
	off_t			size_;
	
	ssize_t fileCopy_(off_t offset, size_t size, uint8_t* to, const uint8_t* from);
public:
	CMappedFile(const char* fileName);
	~CMappedFile();

	CFileRegion* map(off_t offset, off_t size, void** address);
	off_t getSize();
	ssize_t read(off_t offset, size_t size, uint8_t* buf);
	ssize_t write(off_t offset, size_t size, const uint8_t* buf);
	bool isValid();
	
};

#endif


