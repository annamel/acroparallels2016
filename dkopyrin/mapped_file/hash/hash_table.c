#include <stdio.h>
#include <stdlib.h>
#include "hash_table.h"

#define COLOR(x) "\x1B[35m"x"\x1B[0m"
#define LOGCOLOR(x) COLOR("%s: ")x, __func__
#include "../logger/log.h"
const hashtable_value_t hashtable_value_nil = {.ptr = NULL, .state = -1};

struct hashtable *hashtable_init(struct hashtable *hashtable, int size) {
	LOG(INFO, "hashtable_init called\n");
	int i;

	if (size < 1) return NULL;

	/* Allocate pointers to the head nodes. */
	if ((hashtable->table = malloc(sizeof(struct entry) * size)) == NULL){
		LOG(FATAL, "Failed to alloc table\n");
		return NULL;
	}

	for (i = 0; i < size; i++) {
		hashtable->table[i] = NULL;
	}

	hashtable->size = size;

	return hashtable;
}

void hashtable_finalize(struct hashtable *ht) {
	struct entry *newpair = NULL;
	struct entry *next = NULL;
	struct entry *last = NULL;
	int bin = 0;
	for (bin = 0; bin < ht -> size; bin++) {
		for (next = ht -> table[bin]; next; ) {
			last = next;
			next = next -> next;
			free(last);
		}
	}
	free(ht -> table);
	return;
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

  	//TODO: Add all value to logs
  	LOG(DEBUG, "Adding value with key %d to bin %d, value=%d\n", key, bin, value);
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
	int bin = -1;
	struct entry *next = NULL;

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
	return hashtable_value_nil;
}
