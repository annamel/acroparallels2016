#include <mapped_file.h>
#include <cstdio>
#include <vector>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <cstring>
#include <sys/time.h>
#include <errno.h>

#define TEST_SIZE 100000
#define MAJOR_ITERATIONS 1
#define MINOR_ITERATIONS 1000

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
		unlink("test0");								\
		unlink("test1");								\
		unlink("test2");								\
		exit(-1);										\
	}													\
} while (0)

int main()
{
	unlink("test0");
	unlink("test1");
	unlink("test2");
	
	sleep(1);
	
	int f0 = open("test0", O_RDWR | O_CREAT, 0777);
	int f1 = open("test1", O_RDWR | O_CREAT, 0777);
	int f2 = open("test2", O_RDWR | O_CREAT, 0777);
	CHECK(f0 >= 0 && f1 >= 0 && f2 >= 0);
	CHECK(!ftruncate(f0, TEST_SIZE));
	CHECK(!ftruncate(f1, TEST_SIZE));
	CHECK(!ftruncate(f2, TEST_SIZE));
	CHECK(!close(f1));
	CHECK(!close(f2));
	
	long long time_start = time_ms();

	mf_handle_t mf1 = mf_open("test1");
	CHECK(mf1);
	
	mf_handle_t mf2 = mf_open("test2");	
	CHECK(mf2);
	
	for (int i = 0; i < MAJOR_ITERATIONS; i++)
	{		
		std::vector<mf_mapmem_handle_t> regions;
		
		for (int j = 0; j < MINOR_ITERATIONS; j++)
		{
			ssize_t offset = rand() % (TEST_SIZE - 1);
			ssize_t size = 1 + rand() % (TEST_SIZE - offset);
			uint8_t* buf = new uint8_t[size];
			for (int k = 0; k < size; k++)
				buf[k] = (uint8_t) rand();
			
			CHECK(lseek(f0, offset, SEEK_SET) == offset);
			CHECK(write(f0, buf, size) == size);
			
			mf_mapmem_handle_t handle = NULL;
			void* data = mf_map(mf1, offset, size, &handle);
			CHECK(data);
			regions.push_back(handle);
			memcpy(data, buf, size);
			
			CHECK(mf_write(mf2, buf, size, offset) == size);
			
			delete[] buf;
		}
		
		CHECK(mf_file_size(mf1) == TEST_SIZE);
		CHECK(mf_file_size(mf2) == TEST_SIZE);
		
		for (auto handle : regions)
			CHECK(!mf_unmap(mf1, handle));
	}
	
	CHECK(!close(f0));
	CHECK(!mf_close(mf1));
	CHECK(!mf_close(mf2));
	
	printf("Total time: %lld ms.\n", time_ms() - time_start);
	
	FILE* o0 = popen("md5sum -b test0", "r");
	FILE* o1 = popen("md5sum -b test1", "r");
	FILE* o2 = popen("md5sum -b test2", "r");
	CHECK(o0);
	CHECK(o1);
	CHECK(o2);
	
	char checksum0[80];
	char checksum1[80];
	char checksum2[80];
	CHECK(fscanf(o0, "%s", checksum0) == 1);
	CHECK(fscanf(o1, "%s", checksum1) == 1);
	CHECK(fscanf(o2, "%s", checksum2) == 1);
	
	int err = 0;
	if (strcmp(checksum0, checksum1))
		err |= 1;
		
	if (strcmp(checksum0, checksum2))
		err |= 2;
		
	fclose(o0);
	fclose(o1);
	fclose(o2);
	
	unlink("test0");
	unlink("test1");
	unlink("test2");
	
	return err;
}
