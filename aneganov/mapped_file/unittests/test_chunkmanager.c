#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "chunk_manager.h"
#include "log.h"

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while(0)

#define FILE_NAME "file"

static void print_usage(const char * const program_name, FILE * const stream, const int exit_code) {
    fprintf(stream, "Usage: %s options\n", program_name);
    fprintf(stream,
        "   -h   --help                    Display this usage information\n"
        "   -L   --log-file     filename   Set log file\n"
        "   -d   --log-level    lvl        Set log level to lvl\n");
    exit(exit_code);
}

void bus_handler(int signum __attribute__ ((unused))) {
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

	int fd = open(FILE_NAME, O_RDWR, 0666);
	if(fd == -1)
		handle_error("open");

	chpool_t *cpool = NULL;
	int err = chpool_construct(fd, PROT_READ | PROT_WRITE, &cpool);
	log_write(err ? LOG_FATAL : LOG_INFO, "chpool_construct: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	chunk_t * chunk;
	err = chunk_acquire(cpool, 20, 8000, &chunk);
	log_write(err ? LOG_ERR : LOG_INFO, "chunk_acquire: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	chunk_t * chunk1;
	err = chunk_find(cpool, 30, 200, &chunk1);
	log_write(err ? LOG_ERR : LOG_INFO, "chunk_find: %s\n", strerror(err));
	log_write( (chunk != chunk1) ? LOG_ERR : LOG_INFO, "value = %p, expected value = %p\n", chunk1, chunk );
	nr_tests++;
	if(err || chunk != chunk1) nr_faults++;

	char *buf;
	char buf1[11] = {0};
	int size = pread(fd, buf1, 10, 40);
	if(size == 10) {
		err =  chunk_get_mem(chunk, 40, (void **)&buf);
		int corruption = 0;
		for(int i = 0; i < 10; i++)
			if( buf[i] != buf1[i] || buf[i] != 'a' ) corruption = 1;
		buf[10] = 0;
		log_write(err ? LOG_ERR : LOG_INFO, "chunk_get_mem: %s\n", strerror(err));
		log_write(LOG_DEBUG, "buf = %p, buf = %s, buf1 = %s\n", buf, buf, buf1);
		log_write(corruption ? LOG_ERR : LOG_INFO, "corruption: %s\n", corruption ? "True" : "False");
		buf[10] = 'a';
		if(err || corruption) nr_faults++;
	}
	else {
		log_write(LOG_ERR, "cannot read from file\n");
		nr_faults++;
	}
	nr_tests++;

	err = chunk_release(chunk);
	log_write(err ? LOG_ERR : LOG_INFO, "chunk_release: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	chunk_t *chunks[5];

	for(int i = 0; i < 5; i++) {
		err = chunk_acquire(cpool, 20 + 8000*i, 8000, chunks+i);
		log_write(err ? LOG_ERR : LOG_INFO, "chunk_acquire: %s\n", strerror(err));
		nr_tests++;
		if(err) nr_faults++;
	}

	err = chunk_acquire(cpool, 20 + 8000*5, 8000, &chunk);
	log_write( (err) ? LOG_ERR : LOG_INFO, "chunk_acquire: %s\n", strerror(err));
	nr_tests++;
	if(err ) nr_faults++;


	for(int i = 0; i < 5; i++) {
		err = chunk_release(chunks[i]);
		log_write(err ? LOG_ERR : LOG_INFO, "chunk_release: %s\n", strerror(err));
		nr_tests++;
		if(err) nr_faults++;
	}

	err = chunk_acquire(cpool, 20 + 8000*4, 16000, &chunk);
	log_write(err ? LOG_ERR : LOG_INFO, "chunk_acquire: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	err = chpool_destruct(cpool);
	log_write(err ? LOG_FATAL : LOG_INFO, "chpool_destruct: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	fd = open(FILE_NAME, O_RDWR, 0666);
	if(fd == -1)
		handle_error("open");

	err = chpool_construct(fd, PROT_READ | PROT_WRITE, &cpool);
	log_write(err ? LOG_FATAL : LOG_INFO, "chpool_construct: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	err = chunk_acquire(cpool, 20, 100, &chunk);
	log_write(err ? LOG_ERR : LOG_INFO, "chunk_acquire: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	err = chunk_acquire(cpool, 20, 8000, &chunk);
	log_write(err ? LOG_ERR : LOG_INFO, "chunk_acquire: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	size = pread(fd, buf1, 10, 5000);
	if(size == 10) {
		err =  chunk_get_mem(chunk, 5000, (void **)&buf);
		int corruption = 0;
		for(int i = 0; i < 10; i++)
			if( buf[i] != buf1[i] || buf[i] != 'a' ) corruption = 1;
		buf[10] = 0;
		log_write(err ? LOG_ERR : LOG_INFO, "chunk_get_mem: %s\n", strerror(err));
		log_write(LOG_DEBUG, "buf = %p, buf = %s, buf1 = %s\n", buf, buf, buf1);
		log_write(corruption ? LOG_ERR : LOG_INFO, "corruption: %s\n", corruption ? "True" : "False");
		buf[10] = 'a';
		if(err || corruption) nr_faults++;
	}
	else {
		log_write(LOG_ERR, "cannot read from file\n");
		nr_faults++;
	}
	nr_tests++;

	int fd1 = chpool_fd(cpool);
	log_write( (fd1 != fd) ? LOG_ERR : LOG_INFO, "chpool_fd\n" );
	nr_tests++;
	if(fd1 != fd) nr_faults++;

	err = chpool_destruct(cpool);
	log_write(err ? LOG_FATAL : LOG_INFO, "chpool_destruct: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	log_write(LOG_SUMMARY, "%d tests OK, %d tests FAILED\n", nr_tests - nr_faults, nr_faults);

	return 0;
}