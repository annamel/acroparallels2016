/*

 # hashtable header file
 #
 # Lang:     C
 # Author:   okhlopkov
 # Version:  0.4

 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "settings.h"

typedef struct hashtable_pair_t hashtable_pair_t;

typedef struct {
        hashtable_pair_t **arr;
        uint32_t count;
        uint32_t size;
} hashtable_t;


hashtable_t *hashtable_construct (uint32_t size);
uint32_t hashtable_destruct (hashtable_t *hashtable);

uint32_t hashtable_count (hashtable_t *hashtable);
uint32_t hashtable_size (hashtable_t *hashtable);

uint32_t hashtable_add (hashtable_t *hashtable, hkey_t key, hval_t value);
hval_t   hashtable_get (hashtable_t *hashtable, hkey_t key);
uint32_t hashtable_delete (hashtable_t *hashtable, hkey_t key);
