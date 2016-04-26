#include <sys/time.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>

long long time_ms()
{
    struct timeval te; 
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
    return milliseconds;
}

#define CHECK(cond) 									\
do														\
{														\
	if (!(cond))										\
	{													\
		printf("FAILED (%s) at %d :", #cond, __LINE__);	\
		printf("\t\"%s\"\n", strerror(errno));			\
		unlink("test");								\
		unlink("test0");								\
		unlink("test1");								\
		unlink("test2");								\
		exit(-1);										\
	}													\
} while (0)

#define lrand() (size_t(rand()) * RAND_MAX * RAND_MAX + size_t(rand()) * RAND_MAX + size_t(rand()))
