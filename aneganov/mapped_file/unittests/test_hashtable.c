#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

#include "hashtable.h"
#include "log.h"

static int simple_cmp(hkey_t x, hkey_t y) {
	return (x > y) ? 1 : (x < y) ? -1 : 0;
}

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

	int nr_tests = 0;
	int nr_faults = 0;

	char data[65] = "abcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefgh";
	void *data1[64] = {0};

	hashtable_t *ht;

	int err = hashtable_construct(simple_cmp, simple_hash, &ht);
	log_write(err ? LOG_FATAL : LOG_INFO, "hashtable_construct: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	for(int i = 0; i < 64; i++) {
		err = hashtable_add(ht, i/2 + i*HASHTABLE_SIZE, data+i);
		log_write(err ? LOG_ERR : LOG_INFO, "hashtable_add: %s\n", strerror(err));
		nr_tests++;
		if(err) nr_faults++;
	}

	for(int i = 0; i < 64; i++) {
		err = hashtable_get(ht, i/2 + i*HASHTABLE_SIZE, data1+i);
		log_write(err ? LOG_ERR : LOG_INFO, "hashtable_get: %s\n", strerror(err));
		log_write( (data1[i] != data+i) ? LOG_ERR : LOG_INFO, "value is %p, expected value is %p\n", data1[i], data+i);
		nr_tests++;
		if(err || data1[i] != data+i) nr_faults++;
	}

	for(int i = 0; i < 64; i++) {
		err = hashtable_get(ht, i+1, data1+i);
		log_write( (err != ENOKEY) ? LOG_ERR : LOG_INFO, "hashtable_get: %s\n", strerror(err));
		nr_tests++;
		if(err != ENOKEY) nr_faults++;
	}

	for(int i = 0; i < 32; i++) {
		err = hashtable_del(ht, i/2 + i*HASHTABLE_SIZE);
		log_write(err ? LOG_ERR : LOG_INFO, "hashtable_del: %s\n", strerror(err));
		nr_tests++;
		if(err) nr_faults++;
	}

	for(int i = 32; i < 64; i++) {
		err = hashtable_get(ht, i/2 + i*HASHTABLE_SIZE, data1+i);
		log_write(err ? LOG_ERR : LOG_INFO, "hashtable_get: %s\n", strerror(err));
		log_write( (data1[i] != data+i) ? LOG_ERR : LOG_INFO, "value is %p, expected value is %p\n", data1[i], data+i);
		nr_tests++;
		if(err || data1[i] != data+i) nr_faults++;
	}

	err = hashtable_del(ht, 1);
	log_write( (err != ENOKEY) ? LOG_ERR : LOG_INFO, "hashtable_del: %s\n", strerror(err));
	nr_tests++;
	if(err != ENOKEY) nr_faults++;

	for(int i = 0; i < 128; i++) {
		err = hashtable_add(ht, (i+(i%7))*HASHTABLE_SIZE + 3, data);
		log_write(err ? LOG_ERR : LOG_INFO, "hashtable_add: %s\n", strerror(err));
		nr_tests++;
		if(err) nr_faults++;
	}

	err = hashtable_add(ht, (30+(30%7))*HASHTABLE_SIZE + 3, data);
	log_write(err ? LOG_ERR : LOG_INFO, "hashtable_add: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	for(int i = 0; i < 128; i++) {
		err = hashtable_get(ht, (i+(i%7))*HASHTABLE_SIZE + 3, data1);
		log_write(err ? LOG_ERR : LOG_INFO, "hashtable_get: %s\n", strerror(err));
		log_write( (data1[0] != data) ? LOG_ERR : LOG_INFO, "value is %p, expected value is %p\n", data1[0], data);
		nr_tests++;
		if(err || (data1[0] != data) ) nr_faults++;
	}

	err = hashtable_destruct(&ht);
	log_write(err ? LOG_FATAL : LOG_INFO, "hashtable_destruct: %s\n", strerror(err));
	nr_tests++;
	if(err) nr_faults++;

	log_write(LOG_SUMMARY, "%d tests OK, %d tests FAILED\n", nr_tests - nr_faults, nr_faults);

	return 0;
}