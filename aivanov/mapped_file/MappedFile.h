#ifndef __MAPPEDFILE__
#define __MAPPEDFILE__

#include "FileRegion.h"


class CMappedFile
{

private:
	int				desc_;
	CFileRegion		root_;
					
public:
	CMappedFile(const char* fileName);
	~CMappedFile();

	CFileRegion* map(off_t offset, off_t size, void** address);
	off_t getSize();
	ssize_t read(off_t offset, size_t size, void *buf);
	ssize_t write(off_t offset, size_t size, const void *buf);
	bool isValid();
	
};

#endif


