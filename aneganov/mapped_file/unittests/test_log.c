#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log.h"

static void print_usage(const char * const program_name, FILE * const stream) {
	fprintf(stream, "Usage: %s --log-file=filename --log-level=lvl\n", program_name);
	fprintf(stream,
		"   -h   --help                   Display this usage information\n"
		"   -L   --log-file    filename   Set a file for logging\n"
		"   -l   --log-level              Set a log level\n"
	    "\n"
		"Possible log levels:\n"
        "LOG_FATAL   0\n"
        "LOG_ERR     1\n"
        "LOG_WARN    2\n"
        "LOG_INFO    3\n"
        "LOG_DEBUG   4\n");
}

int main(int argc, char * argv[]) {
	const char * const short_options = "hl:L:";

	const struct option long_options[] = {
		{"help",      0, NULL, 'h'},
		{"log-level", 1, NULL, 'l'},
		{"log-file",  1, NULL, 'L'},
		{NULL,        0, NULL,  0}
	};

	int option = 0;

	while( (option = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
		switch(option) {
			case 'h':
				print_usage(argv[0], stdout);
				exit(EXIT_SUCCESS);
				break;
			case 'l':
				log_configure(NULL, atoi(optarg));
				break;
			case 'L':
				log_configure(optarg, LOG_DUMMY);
				break;
			default:
				print_usage(argv[0], stderr);
				exit(EXIT_FAILURE);
				break;
		}
	}

	log_write(LOG_INFO, "Hello world!\n");
	log_write(LOG_DEBUG, "option = %d\n", option);
	
	int i = 0;
	for(i=0; i< 15; i++) {
		log_write(LOG_INFO, "Hello world!\n");
	}

	log_write(LOG_FATAL, "Oops!\n");

	exit(EXIT_SUCCESS);
}
