#define LOGLEVEL_STRING_START 9
#define MAX_LOG_NAME 256

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "log.h"

void log_start (int argc, char * argv[]) {
	char log_name_base[MAX_LOG_NAME];
	sprintf(log_name_base, "%s.log", argv[0]);

	int fd = 0;
	fd = open(log_name_base, O_WRONLY | O_APPEND | O_CREAT, 0777);
	if (fd < 0) {
		//This is euristics, but this might work
		char log_name[MAX_MSG_SIZE];
		int i;
		for (i = 0; i < 100; i++){
			sprintf(log_name, "%s%d", log_name_base, i);
			fd = open(log_name, O_WRONLY | O_APPEND | O_CREAT, 0777);
			if (fd >= 0) break;
		}
		//File can't be opened, writing to stderr
		if (i == 100) {
			fprintf(stderr, "[!]Can't open file!\n");
			fd = 2;
		}
	}
	ring_buf_init(&log_ring_buf, fd);
}
void log_clean (void) {
	ring_buf_finalize(&log_ring_buf);
}
