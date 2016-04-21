#include <mapped_file.h>
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

long long time_ms()
{
    struct timeval te; 
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
    return milliseconds;
}

typedef long long (*performance_test)();

#ifndef max
    #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
    #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define ASSERT(cond) 									\
do														\
{														\
	if (!(cond))										\
	{													\
		printf("FAILED %s at %s:%d (%s):", #cond, 		\
			__FILE__, __LINE__, __FUNCTION__);			\
		printf("\t\"%s\"\n", strerror(errno));			\
		return -1;										\
	}													\
} while (0)

#define FILE_SIZE (4096*1024)
#define NUM_MAPS 40000

long long performance_test1()
{
	long long time = time_ms();

	mf_handle_t mf = mf_open("file.txt", 0);
	ASSERT(mf);

	int i;
	for (i = 0; i < FILE_SIZE; i++)
	{
		char c;
		mf_read(mf, i, 1, &c);
	}

	ASSERT(!mf_close(mf));

	return time_ms() - time;
}

long long performance_test2()
{
	long long time = time_ms();

	mf_handle_t mf = mf_open("file.txt", 0);
	ASSERT(mf);

	mf_mapmem_t* mapmems[NUM_MAPS];
	int i;
	for (i = 0; i < NUM_MAPS; i++)
	{
		mf_map(mf, 10, 1000);
	}

	ASSERT(!mf_close(mf));

	return time_ms() - time;
}

int main()
{
	int fd = open("file.txt", O_RDWR | O_CREAT, 0777);
	ASSERT(fd != -1);
	ASSERT(!ftruncate(fd, FILE_SIZE));
	ASSERT(!close(fd));

	performance_test tests[] = { performance_test1, performance_test2 };

	printf("MAPPED FILE TESTS\n");

	int i;
	for (i = 0; i < sizeof (tests) / sizeof (performance_test); i++)
	{
		long long result = tests[i]();

		printf("PERFORMANCE TEST %d: %lldms\n", i + 1, result);
	}
	printf("\n");

	remove("file.txt");

	return 0;
}
