#include <signal.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "mapped_file.h"
#include "../libmf/log.h" //style for testing
#include "../libmf/mfdef.h"

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while(0)

static void print_usage(const char * const program_name, FILE * const stream, const int exit_code) {
    fprintf(stream, "Usage: %s -f file [options]\n", program_name);
    fprintf(stream,
        "   -h   --help                    Display this usage information\n"
	"    			filename   File for testing\n"
        "   -L   --log-file     filename   Set log file\n"
        "   -d   --log-level    lvl        Set log level to lvl: fatal = 1, error = 2, warning = 3, info = 4, debug = 5\n");
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

    char *pathname = argv[1];

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

	if( pathname == NULL )
		print_usage(argv[0], stderr, EXIT_FAILURE);

	struct sigaction act = {.sa_handler = bus_handler, .sa_flags = 0};

	if(sigemptyset(&act.sa_mask) == -1)
		handle_error("sigemptyset");

	if(sigaction(SIGBUS, &act, NULL) == -1)
		handle_error("sigaction");

	int nr_tests = 0;
	int nr_faults = 0;
	int corruption = 0;

	int fd = open(pathname, O_RDWR, 0666);
	if(fd == -1)
		handle_error("open");

	mf_handle_t mf = mf_open(pathname);
	log_write(mf == MF_OPEN_FAILED ? LOG_FATAL : LOG_INFO, "mf_open: %s\n", strerror(errno));
	nr_tests++;
	if(mf == MF_OPEN_FAILED) {
		nr_faults++;
		goto done;
	}

	off_t file_size = mf_file_size(mf);
	log_write(file_size == -1 ? LOG_FATAL : LOG_INFO, "mf_file_size: %s\n", strerror(errno));
	nr_tests++;
	if(file_size == -1) {
		nr_faults++;
		goto done;
	}
	if(file_size == 0) {
		goto done;
	}

	char  buf[4096*2] = {0};
	char buf1[4096*2] = {0};
	memset(buf,  0, 4096*2);
	memset(buf1, 0, 4096*2);

	ssize_t pread_bytes = pread(fd, buf1, 6685, 1);
	if(pread_bytes == -1)
		handle_error("pread");

	mf_mapmem_handle_t mm_handle;
	size_t sz = min(pread_bytes, 1337);
	void *ptr = mf_map(mf, 1, sz, &mm_handle);
	nr_tests++;
	log_write(!ptr ? LOG_ERR : LOG_INFO, "mf_map: %s\n", strerror(errno));
	if(!ptr ){
		nr_faults++;
		goto done;
	}
	if( memcmp(ptr, buf1, sz) )
		corruption = 1;
	log_write(corruption ? LOG_ERR : LOG_INFO, "corruption: %s\n", corruption ? "True" : "False");
	if(corruption) nr_faults++;

	ssize_t read_bytes = mf_read(mf, buf, 6685, 1);
	log_write(read_bytes == -1 ? LOG_ERR : LOG_INFO, "mf_read: %s\n", strerror(errno));
	log_write(read_bytes != pread_bytes ? LOG_ERR : LOG_INFO, "read_bytes = %zd, pread_bytes = %zd\n", read_bytes, pread_bytes);
	if( memcmp(buf, buf1, 6685) )
		corruption = 1;
	log_write(corruption ? LOG_ERR : LOG_INFO, "corruption: %s\n", corruption ? "True" : "False");
	if(read_bytes == -1 || read_bytes != pread_bytes || corruption)
		nr_faults++;
	nr_tests++;
	
	int err = mf_unmap(mf, mm_handle);
	log_write(err ? LOG_ERR : LOG_INFO, "mf_unmap: %s\n", strerror(errno));
	nr_tests++;
	if(err) nr_faults++;

	memset(buf, 'c', 4096*2);

#if 0
	ssize_t written_bytes = mf_write(mf, buf, 6685, 1);
	log_write(written_bytes == -1 ? LOG_ERR : LOG_INFO, "mf_write: %s\n", strerror(errno));
	log_write(written_bytes != 6685 ? LOG_WARN : LOG_INFO, "written_bytes = %zd, expected %d\n", written_bytes, 6685);

	corruption = 0;
	pread_bytes = pread(fd, buf1, 6685, 1);
	if(pread_bytes == -1)
		handle_error("pread");
	sz = min(pread_bytes, 6685);
	if( memcmp(buf, buf1, sz) )
		corruption = 1;
	log_write(corruption ? LOG_ERR : LOG_INFO, "corruption: %s\n", corruption ? "True" : "False");
	if( written_bytes != 6685 || corruption ) nr_faults++;
	nr_tests++;
#endif

	err = mf_close(mf);
	log_write(err ? LOG_ERR : LOG_INFO, "__mf_close: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

done:
	log_write(LOG_SUMMARY, "%d tests OK, %d tests FAILED\n", nr_tests - nr_faults, nr_faults);
	return nr_faults ? 1 : 0;
}
