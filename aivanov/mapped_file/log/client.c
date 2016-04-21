#define _LOG_FLUSH 1

#include "log.h"
#include <stdlib.h>

int main()
{
	int i = 0;
	for (i = 0; i < 3; i++)
	{
			
		fork();
	}


	srand(time(0));
	
	if (rand() % 2)
		init_logger_client("Frank");
			
	LOGE("Hello, I'm an %s!\n", "error");
	LOGI("Info says 0x%X", 653);
	LOGW("What?");
}
