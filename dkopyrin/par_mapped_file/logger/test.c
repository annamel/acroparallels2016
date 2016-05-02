#include "log.h"
#include <stdio.h>

int main() {
	int i = 0;
  	LOG(INFO, "I\n");
	LOG(WARN, "am\n");
	LOG(DEBUG, "testing\n");
	LOG(ERROR, "stuff\n");
	LOG(INFO, "This msg should be after \"stuff\"!\n");
	for (i = 0; i < 250; i++)
		LOG(DEBUG, "Hi %d\n", i);
  	LOG(FATAL, "FATAL ERROR!\n");
	return 0;
}
