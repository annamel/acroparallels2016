#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "log.h"

#define NUM_THREADS 8

void* thread_func(void* data)
{
	logi("Hello!");
	logi("This is me!");
}

int main()
{
	log_init(LOG_ALL, true);

	pthread_t thread[NUM_THREADS];

	int i;
	for (i = 0; i < NUM_THREADS; i++)
	{
		int ret = pthread_create(&thread[i], NULL, thread_func, NULL);
		assert(!ret);
	}

	log_debug("5+2=%d", 5+2);
	logi("Info");
	logw("WARNING");
	loge("ERRROR!!!");

	for (i = 0; i < NUM_THREADS; i++)
	{
		int ret = pthread_join(thread[i], NULL);
		assert(!ret);
	}

	log_destroy();
	return 0;
}
