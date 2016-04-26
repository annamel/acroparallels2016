#include "tests.h"

#define FILE_SIZE (4096*1024)
#define NUM_MAPS 500000

int main()
{
	int fd = open("testfile", O_RDWR | O_CREAT, 0777);
	CHECK(fd != -1);
	CHECK(!ftruncate(fd, FILE_SIZE));
	CHECK(!close(fd));

	long long time = time_ms();

	mf_handle_t mf = mf_open("testfile");
	CHECK(mf != MF_OPEN_FAILED);

	mf_mapmem_handle_t* mapmems = malloc(sizeof (mf_mapmem_handle_t) * NUM_MAPS);
	int i;
	for (i = 0; i < NUM_MAPS; i++)
	{
		CHECK(mf_map(mf, 10 + i, 1000, &mapmems[i]) != MF_MAP_FAILED);
	}
	for (i = 0; i < NUM_MAPS; i++)
		CHECK(mf_unmap(mf, mapmems[i]) != -1);
	free(mapmems);

	CHECK(!mf_close(mf));

	printf("PERFORMANCE TEST 2: %lldms\n", time_ms() - time);

	remove("testfile");

	return 0;
}
