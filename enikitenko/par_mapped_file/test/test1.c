#include "../../mapped_file/test/tests.h"

#define FILE_SIZE (32*1024*1024)

typedef struct
{
	int current_thread;
	int num_threads;
	mf_handle_t mf;
	int fail;
} thread_data_t;

void* thread_func(void* data)
{
	thread_data_t* thread_data = (thread_data_t*) data;

	int start = FILE_SIZE * thread_data->current_thread / thread_data->num_threads;
	int end = FILE_SIZE * (thread_data->current_thread + 1) / thread_data->num_threads;
	int i;
	for (i = start; i < end; i++)
	{
		char c;
		CHECK_THREAD(mf_read(thread_data->mf, &c, 1, i) == 1);
	}

	return NULL;
}

int main(int argc, const char* argv[])
{
	int num_threads = atoi(argv[1]);
	assert(num_threads > 0);

	int fd = open("testfile", O_RDWR | O_CREAT, 0777);
	CHECK(fd != -1);
	CHECK(!ftruncate(fd, FILE_SIZE));
	CHECK(!close(fd));

	long long time = time_ms();
	mf_handle_t mf = mf_open("testfile");
	CHECK(mf != MF_OPEN_FAILED);

	pthread_t* threads = malloc(num_threads);
	assert(threads);
	thread_data_t* threads_data = malloc(num_threads);
	assert(threads_data);

	int i;
	for (i = 0; i < num_threads; i++)
	{
		threads_data[i].current_thread = i;
		threads_data[i].num_threads = num_threads;
		threads_data[i].mf = mf;
		threads_data[i].fail = 0;
		pthread_create(&threads[i], NULL, thread_func, &threads_data[i]);
	}

	for (i = 0; i < num_threads; i++)
	{
		CHECK(!threads_data[i].fail);
		pthread_join(threads[i], NULL);
	}

	free(threads);

	CHECK(!mf_close(mf));

	printf("MULTITHREAD TEST 1: %lldms\n", time_ms() - time);

	remove("testfile");

	return 0;
}
