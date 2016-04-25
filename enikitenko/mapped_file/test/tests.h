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
#include <errno.h>

long long time_ms()
{
    struct timeval te; 
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
    return milliseconds;
}

#ifndef max
    #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
    #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define CHECK(cond) 									\
do														\
{														\
	if (!(cond))										\
	{													\
		printf("FAILED %s at %s:%d (%s):", #cond, 		\
			__FILE__, __LINE__, __FUNCTION__);			\
		printf(" %s\n", strerror(errno));				\
		return -1;										\
	}													\
} while (0)
