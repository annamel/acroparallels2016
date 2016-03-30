#include "log_ring_buf.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <execinfo.h>
#include <stdarg.h>

#define BACKTRACE_SIZE 128

//This array corresponds to enum log_level
char loglevel_chars[6] = {'!', 'I', 'D', 'W', 'E', 'F'};

void print_backtrace(struct ring_buf *rb){
	void *array[BACKTRACE_SIZE];
	size_t size;

	size = backtrace (array, BACKTRACE_SIZE);
	backtrace_symbols_fd (array, size, rb -> fd);
}

int ring_buf_init (struct ring_buf *rb, int fd) {
	assert(rb);

	rb -> next_event = 0;
	rb -> next_flush = 0;
	rb -> fd = fd;
	memset (rb -> events, 0, RING_BUF_SIZE * sizeof(struct event));

	return 0;
}

int ring_buf_flush(struct ring_buf *rb) {
	/* Normal of buffers looks like this
	 * Y Y Y Y Y Y N N N N N N N Y Y Y
	 *                           ^
	 * Y - events to flush
	 * N - events not to flush (LOGLEVEL_NOT_USED)
	 * next_flush ptr
	 * According to such structure we only need to flush until we don't
	 * hit N event. As flushed events will turn to N such cycle won't
	 * become infinite */

	assert(rb);
	struct event *cur_event;
	while ((cur_event = rb -> events + rb -> next_flush) -> level != LOGLEVEL_NOT_USED) {
		//If write failed we can try it again next time
		int ret = -1;
		char cooked_msg[MAX_MSG_SIZE + 10];
		int msg_size = snprintf(cooked_msg, MAX_MSG_SIZE + 9, "[%c] %s", loglevel_chars[cur_event -> level], cur_event -> msg);
		if ((ret = write(rb -> fd, cooked_msg, msg_size)) != msg_size) {
			if (ret == -1 || ret == 0){
				return 1;
			}else{
				//Actually something was written, so lets skip this msg
				if (cur_event -> level >= LOGLEVEL_FATAL)
					print_backtrace(rb);
				cur_event -> level = LOGLEVEL_NOT_USED;
				rb -> next_flush++;
				return 2;
			}
		}

		if (cur_event -> level >= LOGLEVEL_FATAL)
			print_backtrace(rb);
		cur_event -> level = LOGLEVEL_NOT_USED;
		rb -> next_flush++;
	}

	return 0;
}

//This attribute makes finalize always be called after main
int ring_buf_finalize(struct ring_buf * rb) {
	assert(rb);
	//User want to stop work with program - let's flush buffers not to lose logs
	ring_buf_flush (rb);
	return 0;
}

int ring_buf_add_event(struct ring_buf *rb, enum log_level level, pid_t pid, const char *fmt, ...) {
	assert(rb);
	assert(level != LOGLEVEL_NOT_USED);

	rb -> events[rb -> next_event].pid = pid;
	rb -> events[rb -> next_event].level = level;

	//Load all arguments after fmt to make nice output
	va_list argptr;
	va_start(argptr, fmt);
	//Lets be secure here - no overflow is allowed!
	vsnprintf(rb -> events[rb -> next_event].msg, MAX_MSG_SIZE-1, fmt, argptr);
	va_end(argptr);

#ifdef DEBUG_STDERRLOG
	fprintf(stderr, "[%c] %s", loglevel_chars[level], rb -> events[rb -> next_event].msg);
#endif

	/* In case of overwriting of buffer we can lose the right order.
	 * Newer messages should move next_flush in order to events be
	 * in a correct time order. This can be achieved by synchronizing next_event
	 * and next_flush. Such check is made by using the fact the ALL of
	 * events are in this case are to be flushed => next_event is to be flushed
	 * and by default both pointers are synchronized so we just have to keep
	 * both pointers equal */
	if (rb -> next_event == rb -> next_flush && rb -> events[rb -> next_event + 1].level != LOGLEVEL_NOT_USED)
		rb -> next_flush++;
	rb -> next_event++;

	/* Some error or worse happened - developer MUST get as many logs before
	 * error as we can so let's call flush to preserve logs */
	if (level >= LOGLEVEL_ERROR)
		ring_buf_flush(rb);
	return 0;
}

