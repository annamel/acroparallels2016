#ifndef __HASH_TABLE_H_
#define __HASH_TABLE_H_
#include <stdint.h>
#include <stdlib.h>

typedef uint32_t hashtable_key_t;
typedef struct {
	void *ptr;
	int state;
} hashtable_value_t;

struct entry {
	hashtable_key_t key;
	hashtable_value_t value;
	struct entry *next;
};

struct hashtable {
	int size;
	struct entry **table;
};

int hashtable_hash (struct hashtable *hashtable, hashtable_key_t key);
struct entry *hashtable_newpair (hashtable_key_t key, hashtable_value_t value);
void hashtable_set (struct hashtable *hashtable, hashtable_key_t key, hashtable_value_t value);
hashtable_value_t hashtable_get (struct hashtable *hashtable, hashtable_key_t key);

struct hashtable *hashtable_init (struct hashtable *mf, int size);
void hashtable_finalize (struct hashtable *mf);

#endif
