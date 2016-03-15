#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "log.h"

class test
{
public:

	void print()
	{
		loge("Hello from class!");
	}
};

int main()
{
	log_init(LOG_ALL, true);

	test t;
	t.print();

	log_destroy();
	return 0;
}
