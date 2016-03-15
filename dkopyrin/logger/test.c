#include "log.h"
#include <stdio.h>

int main() {
	int i = 0;
  	LOG_INFO("I\n");
	LOG_WARN("am\n");
	LOG_DEBUG("testing\n");
	LOG_ERROR("stuff\n");
	LOG_INFO("This msg should be after \"stuff\"!\n");
	for (i = 0; i < 250; i++)
		LOG_DEBUG("Hi %d\n", i);
  	LOG_FATAL("FATAL ERROR!\n");
	return 0;
}
