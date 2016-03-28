#include <stdio.h>
#include <stdlib.h>
#include "hash_table.h"
#include "../logger/log.h"

struct hashtable *hashtable_create(int size) {
	LOG(INFO, "hashtable_create called\n");
	struct hashtable *hashtable = NULL;
	int i;

	if (size < 1) return NULL;
	if ((hashtable = malloc(sizeof(struct hashtable))) == NULL){
		LOG(FATAL, "Failed to alloc hashtable\n");
		return NULL;
	}

	/* Allocate pointers to the head nodes. */
	if ((hashtable->table = malloc(sizeof(struct entry) * size)) == NULL){
		LOG(FATAL, "Failed to alloc head\n");
		return NULL;
	}

	for (i = 0; i < size; i++) {
		hashtable->table[i] = NULL;
	}

	hashtable->size = size;

	return hashtable;
}

int hashtable_hash(struct hashtable *hashtable, hashtable_key_t a) {
	LOG(INFO, "hashtable_hash called\n");
	a += ~(a<<15);
	a ^=  (a>>10);
	a +=  (a<<3);
	a ^=  (a>>6);
	a += ~(a<<11);
	a ^=  (a>>16);
	return a % hashtable->size;
}

/* Create a key-value pair. */
struct entry *hashtable_newpair(hashtable_key_t key, hashtable_value_t value) {
	LOG(INFO, "hashtable_newpair called\n");
	struct entry *newpair;

	if((newpair = malloc(sizeof(struct entry))) == NULL) {
		LOG(FATAL, "Failed to alloc newpair\n");
		return NULL;
	}

	newpair->key = key;
	newpair->value = value;
	newpair->next = NULL;

	return newpair;
}

void hashtable_set(struct hashtable *hashtable, hashtable_key_t key, hashtable_value_t value) {
	LOG(INFO, "hashtable_set called\n");
	int bin = 0;
	struct entry *newpair = NULL;
	struct entry *next = NULL;
	struct entry *last = NULL;

	bin = hashtable_hash(hashtable, key);

	LOG(DEBUG, "Adding value %d with key %d to bin %d\n", value, key, bin);
	for (next = hashtable->table[bin]; next; last = next, next = next -> next) {
		//Key found, replacing
		if (key == next -> key) {
			LOG(DEBUG, "Key found\n");
			next -> value = value;
			return;
		}
	}

	//Pair was not found - add new one
	newpair = hashtable_newpair(key, value);
	if (last == NULL) {
		LOG(DEBUG, "Init bin\n");
		hashtable->table[bin] = newpair;
	}else{
		LOG(DEBUG, "Added new pair\n");
		last->next = newpair;
	}
}

hashtable_value_t hashtable_get(struct hashtable *hashtable, hashtable_key_t key) {
	LOG(INFO, "hashtable_get called\n");
	int bin = 0;
	struct entry *next;

	bin = hashtable_hash(hashtable, key);
	next = hashtable->table[bin];

	LOG(DEBUG, "Searching value with key %d in bin %d\n", key, bin);
	for (next = hashtable->table[bin]; next; next = next -> next) {
		//Key found, replacing
		if (key == next -> key) {
			LOG(DEBUG, "Value found by key %d: %d\n", key, next->value);
			return next -> value;
		}
	}
	LOG(DEBUG, "No value found\n");
	return NULL;
}
