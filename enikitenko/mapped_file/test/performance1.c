#include "tests.h"

#define FILE_SIZE (4096*1024)

int main()
{
	int fd = open("file.txt", O_RDWR | O_CREAT, 0777);
	CHECK(fd != -1);
	CHECK(!ftruncate(fd, FILE_SIZE));
	CHECK(!close(fd));

	long long time = time_ms();

	mf_handle_t mf = mf_open("file.txt");
	CHECK(mf != MF_OPEN_FAILED);

	int i;
	for (i = 0; i < FILE_SIZE; i++)
	{
		char c;
		mf_read(mf, &c, 1, i);
	}

	CHECK(!mf_close(mf));

	printf("PERFORMANCE TEST 1: %lldms\n", time_ms() - time);

	remove("file.txt");

	return 0;
}
