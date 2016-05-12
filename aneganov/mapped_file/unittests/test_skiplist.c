#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>

#include "skiplist.h"
#include "log.h"

#define sampleSize 65536

static void print_usage(const char * const program_name, FILE * const stream, const int exit_code) {
    fprintf(stream, "Usage: %s options\n", program_name);
    fprintf(stream,
        "   -h   --help                    Display this usage information\n"
        "   -L   --log-file     filename   Set log file\n"
        "   -d   --log-level    lvl        Set log level to lvl\n");
    exit(exit_code);
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

	skiplist_t *l;
	off_t keys[sampleSize];
	void* v;

	int err = skiplist_construct(32, &l);
	log_write(err ? LOG_ERR : LOG_INFO, "skiplist_construct: %s\n", strerror(err));
	if(err) {
		return 1;
	}

	for(int k = 0; k < sampleSize; k++) {
		keys[k] = k;
		err = skiplist_add(l, keys[k], (void *)keys[k]);
		log_write(err ? LOG_ERR : LOG_INFO, "skiplist_add (key = %jx): %s\n", keys[k], strerror(err));
		if(err) {
			return 2;
		}
	};


	for(int i = 0; i < 4; i++) {
		for(int k = 0; k < sampleSize; k++) {
			err = skiplist_get(l, keys[k], &v);
			log_write(err ? LOG_ERR : LOG_INFO, "skiplist_get (key = %p, expected = %p): %s\n", v, (void *)keys[k], strerror(err));
			if (err) {
				return 3;
			}
			if (v != (void *)keys[k]) {
				log_write(LOG_ERR, "skiplist_get returned wrong value\n");
				return 4;
			}
		};

		for(int k = 0; k < sampleSize; k++) {
			err = skiplist_del(l, keys[k]);
			log_write(err ? LOG_ERR : LOG_INFO, "skiplist_del (key = %jx): %s\n", keys[k], strerror(err));
			if(err) {
				return 5;
			}

			keys[k] = k;

			err = skiplist_add(l, keys[k], (void *)keys[k]);
			log_write(err ? LOG_ERR : LOG_INFO, "skiplist_add (key = %jx): %s\n", keys[k], strerror(err));
			if(err) {
				return 2;
			}
		};
	};

skiplist_destruct(l);
return 0;
}