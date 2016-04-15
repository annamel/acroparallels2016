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

struct hashtable_pair_t{
        char *key;
        char *value;
        hashtable_pair_t *next;
        hashtable_pair_t *prev;
};

typedef struct {
        hashtable_pair_t *arr;
        uint32_t count;
        uint32_t size;
} hashtable_t;


hashtable_t *hashtable_create_table (uint32_t size);
hashtable_pair_t hashtable_create_pair (char *nkey, char *nvalue);
uint32_t hashtable_delete_table (hashtable_t *hashtable);


uint32_t hashtable_is_empty (hashtable_t *hashtable);
uint32_t hashtable_count (hashtable_t *hashtable);
uint32_t hashtable_size (hashtable_t *hashtable);

/*
   add_pair returns id of pair if it is already added, or adds and returns new id.
   Returns SIZE_EXCEEDED if no more space in table left.
   Can be used with arguments key and value or pair.
 */
uint32_t hashtable_add (hashtable_t *hashtable, char *key, char *value);
uint32_t hashtable_add_pair (hashtable_t *hashtable, hashtable_pair_t pair);

/*
   get_value returns value by key or id if found.
   If no pair found or no table is created then NULL is returned.
 */
char *hashtable_get (hashtable_t *hashtable, char *key);

/*
   get_pair returns pair by key or id if found.
   If no pair found by key or id then NULL is returned.
 */
hashtable_pair_t *hashtable_get_pair (hashtable_t *hashtable, char *key);

/*
   returns 1 if no pair found by key.
 */
uint32_t hashtable_delete (hashtable_t *hashtable, char *key);
