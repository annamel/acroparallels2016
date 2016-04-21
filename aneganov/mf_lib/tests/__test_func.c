#include <signal.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "__mapped_file.h"
#include "log.h"

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while(0)

#define FILE_NAME "file"
#define MAX_MEMORY (4096 * 10)

static void print_usage(const char * const program_name, FILE * const stream, const int exit_code) {
    fprintf(stream, "Usage: %s options\n", program_name);
    fprintf(stream,
        "   -h   --help                    Display this usage information\n"
        "   -L   --log-file     filename   Set log file\n"
        "   -d   --log-level    lvl        Set log level to lvl\n");
    exit(exit_code);
}

void bus_handler() {
	log_write(LOG_ERR, "SIGBUS received: attempted access to a portion of the buffer that does not correspond to the file\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    const char * const short_options = "hL:d:";

    const struct option long_options[] = {
        {"help",         0, NULL, 'h'},
        {"log-file",     1, NULL, 'L'},
        {"log-level",    1, NULL, 'd'},
        {NULL,           0, NULL,  0}
    };

    int ch = 0;
	while ((ch = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
	{
		switch (ch) {
		case 'h':
			print_usage(argv[0], stdout, EXIT_SUCCESS);
			break;
		case 'L':
			log_configure(optarg, LOG_DUMMY);
			break;
		case 'd':
			log_configure(NULL, atoi(optarg));
			break;
		default:
			print_usage(argv[0], stderr, EXIT_FAILURE);
		}
	}

	struct sigaction act = {.sa_handler = bus_handler, .sa_flags = 0};

	if(sigemptyset(&act.sa_mask) == -1)
		handle_error("sigemptyset");

	if(sigaction(SIGBUS, &act, NULL) == -1)
		handle_error("sigaction");

	int nr_tests = 0;
	int nr_faults = 0;

	mf_handle_t mf;

	int err = __mf_open(FILE_NAME, MAX_MEMORY, O_RDWR, 0666, &mf);
	log_write(err ? LOG_ERR : LOG_INFO, "__mf_open: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	char buf[4096+11] = {0};
	ssize_t read_bytes;

	err = __mf_read(mf, 0, 10, &read_bytes, buf);
	log_write(err ? LOG_ERR : LOG_INFO, "__mf_read: %s\n", strerror(err));
	log_write(read_bytes != 10 ? LOG_WARN : LOG_INFO, "read_bytes = %zd\n", read_bytes);
	if(read_bytes == 10) {
		int corruption = 0;
		for(int i = 0; i < 10; i++)
			if( buf[i] != 'a' ) corruption = 1;
		log_write(LOG_DEBUG, "buf = %s\n", buf);
		log_write(corruption ? LOG_ERR : LOG_INFO, "corruption: %s\n", corruption ? "True" : "False");
		if(err || corruption) nr_faults++;
	}
	else {
		log_write(LOG_WARN, "cannot read from file\n");
		nr_faults++;
	}
	nr_tests++;

	void *mem = NULL;
	err = __mf_acquire(mf, 0, 10, &mem);
	log_write(err ? LOG_ERR : LOG_INFO, "__mf_acquire: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	err = __mf_close(mf);
	log_write(err ? LOG_ERR : LOG_INFO, "__mf_close: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	log_write(LOG_SUMMARY, "%d tests OK, %d tests FAILED\n", nr_tests - nr_faults, nr_faults);

	return 0;
}