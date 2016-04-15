/*

 # hashtable header file
 #
 # Lang:     C
 # Author:   okhlopkov
 # Version:  0.1

 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct hashtable_pair_t hashtable_pair_t;

typedef struct {
        hashtable_pair_t *arr;
        uint32_t count;
        uint32_t size;
} hashtable_t;


hashtable_t *hashtable_construct (uint32_t size);
uint32_t hashtable_destruct (hashtable_t *hashtable);

uint32_t hashtable_is_empty (hashtable_t *hashtable);
uint32_t hashtable_count (hashtable_t *hashtable);
uint32_t hashtable_size (hashtable_t *hashtable);

uint32_t hashtable_add (hashtable_t *hashtable, char *key, char *value);
char *hashtable_get (hashtable_t *hashtable, char *key);
uint32_t hashtable_delete (hashtable_t *hashtable, char *key);
