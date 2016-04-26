#include <cstdio>
#include <mapped_file.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>	
#include "test.h"

#define TEST_SIZE 100000000
#define MAX_TOTAL_SIZE (100LL << 40)


int main()
{
	unlink("test");
	
	long long time_start = time_ms();
	
	int f = open("test", O_RDWR | O_CREAT, 0777);
	CHECK(f >= 0);
	CHECK(!ftruncate(f, TEST_SIZE));
	CHECK(!close(f));
	
	mf_handle_t mf = mf_open("test");
	CHECK(mf);
	
	
	size_t total_size = 0;
	
	ssize_t offset = 0;
	std::vector<mf_mapmem_handle_t> regions;
	
	while (total_size < MAX_TOTAL_SIZE)
	{
		offset = lrand() % TEST_SIZE;
		ssize_t size = 1 + (lrand() % MAX_TOTAL_SIZE) % (TEST_SIZE - offset);
		
		mf_mapmem_handle_t handle = NULL;
		void* data = mf_map(mf, offset, size, &handle);
		
		if (!data)
			break;

		regions.push_back(handle);
		total_size += size;
	}
	
	for (auto handle : regions)
		CHECK(!mf_unmap(mf, handle));
	
	CHECK(!mf_close(mf));
	
	printf("Total time: %lld ms.\n", time_ms() - time_start);
	
	unlink("test");
	
	if (total_size >= MAX_TOTAL_SIZE)
		printf("Total memory mapped > %.1lf GB\n", double(MAX_TOTAL_SIZE) / (1 << 30));
	else
		printf("Total memory mapped: %.1lf GB\n", double(total_size) / (1 << 30));
		
	return 0;
}
