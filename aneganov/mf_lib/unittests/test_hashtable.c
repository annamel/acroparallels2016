#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hashtable.h"
#include "log.h"

static int simple_cmp(hkey_t x, hkey_t y) {
	return (x > y) ? 1 : (x < y) ? -1 : 0;
}

int main() {
	char data[65] = "abcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefgh";
	void *data1[64] = {0};

	hashtable_t *ht;

	int err = hashtable_construct(simple_cmp, simple_hash, &ht);
	log_write(LOG_INFO, "hashtable_construct: %s\n", strerror(err));

	for(int i = 0; i < 64; i++) {
		err = hashtable_add(ht, i + i*HASHTABLE_SIZE, data+i);
		log_write(LOG_INFO, "hashtable_add: %s\n", strerror(err));
	}

	for(int i = 0; i < 64; i++) {
		err = hashtable_get(ht, i+i*HASHTABLE_SIZE, data1+i);
		log_write(LOG_INFO, "hashtable_get: %s\n", strerror(err));
		log_write(LOG_INFO, "value is %p, expected value is %p\n", data1[i], data+i);
	}

	for(int i = 0; i < 64; i++) {
		err = hashtable_get(ht, i+1, data1+i);
		log_write(LOG_INFO, "hashtable_get: %s\n", strerror(err));
	}

	for(int i = 0; i < 32; i++) {
		err = hashtable_del(ht, i+i*HASHTABLE_SIZE);
		log_write(LOG_INFO, "hashtable_del: %s\n", strerror(err));
	}

	for(int i = 32; i < 64; i++) {
		err = hashtable_get(ht, i+i*HASHTABLE_SIZE, data1+i);
		log_write(LOG_INFO, "hashtable_get: %s\n", strerror(err));
		log_write(LOG_INFO, "value is %p, expected value is %p\n", data1[i], data+i);
	}

	err = hashtable_destruct(&ht);
	log_write(LOG_INFO, "hashtable_destruct: %s\n", strerror(err));

#ifndef LOG_ON
	if(err) return 1; //for the happiness of the compiler
#endif

	return 0;
}