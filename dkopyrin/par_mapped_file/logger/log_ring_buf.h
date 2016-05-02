#ifndef __LOG_RING_BUF_H
#define __LOG_RING_BUF_H

#include "log_types.h"
#define LOG2_RING_BUF_SIZE 8
#define RING_BUF_SIZE (1 << LOG2_RING_BUF_SIZE)

struct ring_buf {
	struct event events[RING_BUF_SIZE];
	int fd;
	/* Let RING_BUG_SIZE size 2^k, k - number of bits
	 * This helps not to use mod operations and exploit overflow operations
	 * which allows not to make finicky checks of next_event and
	 * next_flush */
	unsigned int next_event: LOG2_RING_BUF_SIZE;
	unsigned int next_flush: LOG2_RING_BUF_SIZE;
};

int ring_buf_init (struct ring_buf *rb, int fd);
int ring_buf_flush(struct ring_buf *rb);
int ring_buf_finalize(struct ring_buf * rb);
int ring_buf_add_event(struct ring_buf *rb, enum log_level level, pid_t pid, const char *fmt, ...);


#endif
