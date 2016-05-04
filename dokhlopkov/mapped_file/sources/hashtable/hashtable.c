/*

 # hashtable source file
 #
 # Lang:     C
 # Author:   okhlopkov
 # Version:  0.5

 */

#include <stdio.h>
#include <stdint.h>

#include "hashtable.h"
#include "hashfunction.h"
#include "../logger/logger.h"

struct hashtable_pair_t{
        hkey_t key;
        hval_t value;
        hashtable_pair_t *next;
        hashtable_pair_t *prev;
};

/* Private methods */

uint32_t hash (hkey_t key);
uint32_t hashtable_check_if_pair_is_good (hashtable_pair_t *pair);

/* Constructors */

hashtable_t *hashtable_construct (uint32_t size) {
    #if defined DEBUG || defined INFO || defined ERROR
        LOG("Creating new Hashtable");
    #endif
    if (size == 0) {
        #if defined ERROR
           LOG("ERROR: hashtable_construct: bad argument:  size = 0");
        #endif
        return NULL;
    }
    #if defined DEBUG
    LOG("  size: %u", size);
    #endif
    hashtable_t *hashtable = (hashtable_t *)malloc(sizeof(hashtable_t));
    if (hashtable == NULL) {
        #if defined ERROR
           LOG("ERROR: hashtable_construct: bad allocation: hashtable");
        #endif
        return NULL;
    }
    //initialising count and size fields.
    hashtable->count = 0;
    hashtable->size = size;
    //allocating memory for data.
    hashtable->arr = (hashtable_pair_t **)calloc(size, sizeof(hashtable_pair_t*));
    if (hashtable->arr == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_construct: bad allocation: hashtable's data");
        #endif
        return NULL;
    }

    #if defined DEBUG || defined INFO
        LOG("  Table %u was created.", (uint32_t)hashtable);
    #endif
    return hashtable;
}

hashtable_pair_t hashtable_create_pair (hkey_t nkey, hval_t nvalue) {
    #if defined DEBUG || defined INFO
        LOG("Creating table pair: key -> value");
    #endif

    if (nvalue == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_create_pair: bad argument:  nvalue");
        #endif
        hashtable_pair_t bad_value = {(hkey_t)EINVAL, (hval_t)EINVAL};
        return bad_value;
    }

    #if defined DEBUG
        LOG("  nkey: %d\n  nvalue: %d", (int)nkey, (int)nvalue);
    #endif

    //creating pair.
    hashtable_pair_t a = {.key = nkey, .value = nvalue, .next = NULL, .prev = NULL};
    #if defined DEBUG
        LOG("  pair created:\n  key:%s\n  value:%s", key_copy, value_copy);
    #endif
    return a;
}

/* Destructor */

uint32_t hashtable_destruct (hashtable_t *hashtable) {
    #if defined DEBUG || defined INFO
    LOG("Deleting table: %u", (uint32_t)hashtable);
    #endif

    //checking arguments.
    if (hashtable == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_delete_table: bad argument:  hashtable");
    #endif
    return EINVAL;
    }
    if (hashtable->arr == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_delete_table: corrupted hashtable:  hashtable->arr");
    #endif
    return EINVAL;
    }

    hashtable_pair_t **array = hashtable->arr;

    //freeing all keys-value pairs.
    uint32_t size = hashtable_size(hashtable);
    for (int i = 0; i < size; i++) {
        if (array[i] != NULL) {
          hashtable_pair_t* next = array[i]->next;
          hashtable_pair_t* current;
          while (next != NULL) {
            current = next;
            next = next->next;
            free(current);
          }
        }
    }

    free (array);
    free (hashtable);

    #if defined DEBUG || defined INFO || defined ERROR
    LOG("Hashtable was successfully destructed.");
    #endif
    return 0;
}

/* Public properties */

uint32_t hashtable_count (hashtable_t *hashtable) {
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_count: bad argument:  hashtable");
        #endif
        return EINVAL;
    }
    #if defined DEBUG
        LOG("--count for: %u", (uint32_t)hashtable);
    #endif
    return hashtable->count;
}

uint32_t hashtable_size (hashtable_t *hashtable) {
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_size: bad argument:  hashtable");
        #endif
        return EINVAL;
    }
    #if defined DEBUG
        LOG("--size for: %u", (uint32_t)hashtable);
    #endif
    return hashtable->size;
}

/* Public methods */
// Addtion
uint32_t hashtable_add_pair (hashtable_t *hashtable, hashtable_pair_t pair) {
    if (hashtable == NULL) {
        #if defined ERROR
          LOG("ERROR: hashtable_add_pair: bad argument:  hashtable");
        #endif
        return EINVAL;
    }
    if (hashtable->arr == NULL) {
        #if defined ERROR
          LOG("ERROR: hashtable_add_pair: corrupted hashtable data:  arr");
        #endif
        return EINVAL;
    }
    if (!hashtable_check_if_pair_is_good (&pair)) return EINVAL;
    #if defined DEBUG || defined INFO
    LOG("Add pair to: %u", (uint32_t)hashtable);
    #endif

    //assigning to new pointer in order to write less code.
    hashtable_pair_t **array = hashtable->arr;

    //i is first found hash of the key.
    uint32_t i = hash(pair.key) % hashtable_size(hashtable);

    if (array[i] == NULL) {
        hashtable_pair_t *new_pair = (hashtable_pair_t *)malloc(sizeof(hashtable_pair_t));
        new_pair->key = pair.key;
        new_pair->value = pair.value;
        new_pair->next = NULL;
        new_pair->prev = NULL;
        array[i] = new_pair;
    } else {
        hashtable_pair_t *current_pair = array[i]->next;
        hashtable_pair_t *prev_pair = array[i];
        while (current_pair != NULL) {
            prev_pair = current_pair;
            current_pair = current_pair->next;
        }
        current_pair = (hashtable_pair_t *)malloc(sizeof(hashtable_pair_t));
        current_pair->key = pair.key;
        current_pair->value = pair.value;
        current_pair->next = NULL;
        current_pair->prev = prev_pair;
        prev_pair->next = current_pair;
    }

    hashtable->count ++;
    #if defined DEBUG
        LOG("  pair added at  %u\n  to table: %u", (uint32_t)i, (uint32_t)hashtable);
    #endif
    return 0;
}

uint32_t hashtable_add (hashtable_t *hashtable, hkey_t key, hval_t value) {
    #if defined DEBUG || defined INFO
        LOG("Add by key value");
    #endif
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_add_pair_by_key_value: bad argument:  hashtable");
        #endif
        return EINVAL;
    }

    #if defined DEBUG
        LOG("  to hashtable: %u\n  with key: %d\n  and value: %d",
                              (uint32_t)hashtable, (int)key, (int)value);
    #endif
    return hashtable_add_pair(hashtable, hashtable_create_pair (key, value));
}

// Getters
hval_t hashtable_get (hashtable_t *hashtable, hkey_t key) {
    #if defined DEBUG || defined INFO
    LOG("Get value by key");
    #endif
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_get_value_by_key: bad argument:  hashtable");
        #endif
        return NULL;
    }

    if (hashtable->arr == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_get_value_by_key: corrupted hashtable:  hashtable->arr");
        #endif
        return NULL;
    }

    #if defined DEBUG
        LOG("  hashtable: %u\n           key: %d", (uint32_t)hashtable, (int)key);
    #endif

    uint32_t i = hash(key) % hashtable_size(hashtable);

    //looking for value.
    int not_found = 1;
    hashtable_pair_t *current_pair = hashtable->arr[i];

    while (current_pair != NULL && not_found) {
        if (current_pair->key == key)
            not_found = 0;
        else
            current_pair = current_pair->next;
    }

    //returning NULL in cases of not found.
    if (not_found) {
        #if defined ERROR
            LOG("ERROR: hashtable_get_value_by_key: element not found");
        #endif
        return NULL;
    }

    if (current_pair->value == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_get: bad value in hashtable->arr\n  at index: %u",
                                (uint32_t)i);
        #endif
        return NULL;
    }

    #if defined DEBUG
        LOG("  hashtable: %u\n    with key: %d\n  and value: %d",
                          (uint32_t)hashtable, (int)key, (int)value_copy);
    #endif
    return current_pair->value;
}

hashtable_pair_t *hashtable_get_pair (hashtable_t *hashtable, hkey_t key) {
  #if defined DEBUG || defined INFO
    LOG("Get pair by key");
  #endif
  if (hashtable == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_pair_by_key: bad argument:  hashtable");
    #endif
    return NULL;
  }

  //retreiving value.
  hval_t nvalue = hashtable_get (hashtable, key);
  if (nvalue == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_pair_by_key: can't get value by key");
    #endif
    return NULL;
  }
  #if defined DEBUG
    LOG("  hashtable: %u\n           key: %d", (uint32_t)hashtable, (int)key);
  #endif

  //creating pair to return.
  hashtable_pair_t *pair = (hashtable_pair_t *)malloc(sizeof(hashtable_pair_t));
  if (pair == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get: bad allocation:  pair");
    #endif
    return NULL;
  }

  pair->key = key;
  pair->value = nvalue;
  #if defined DEBUG
    LOG("  pair was founded at key: %s\n                    value: %d",
                                key, (int)nvalue);
  #endif
  return pair;

}

// Deletion
uint32_t hashtable_delete (hashtable_t *hashtable, hkey_t key) {
    #if defined DEBUG || defined INFO
     LOG("Delete pair by key");
    #endif
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_delete_pair_by_key: bad argument:  hashtable");
        #endif
        return EINVAL;
    }

    if (hashtable->arr == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_delete_pair_by_key: corrupted:  hashtable->arr");
        #endif
        return EINVAL;
    }

    #if defined DEBUG
        LOG("  hashtable: %u\n           key: %d\n",
                            (uint32_t)hashtable, (int)key);
    #endif


    uint32_t i = hash (key) % hashtable_size(hashtable);

    int not_found = 1;
    hashtable_pair_t *current_pair = hashtable->arr[i];

    while (current_pair != NULL && not_found) {
        if (current_pair->key == key)
            not_found = 0;
        else
            current_pair = current_pair->next;
    }

    if (not_found) {
        #if defined ERROR
            LOG("ERROR: hashtable_delete_pair_by_key: pair not found");
        #endif
        return ENODATA;
    }

    if (current_pair->prev == NULL) {
      hashtable->arr[i] = current_pair->next;
    } else {
      current_pair->prev->next = current_pair->next;
      current_pair->next->prev = current_pair->prev;
    }
    free(current_pair);
	  hashtable->count --;
    #if defined DEBUG
        LOG("  pair was successufully deleted.");
    #endif
	return 0;
}

/* Private methods */

uint32_t hashtable_check_if_pair_is_good (hashtable_pair_t *pair) {
    #if defined DEBUG
        LOG("--checking if pair is good");
    #endif
    if (pair == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_check_if_pair_is_good: bad argument:  pair");
        #endif
        return 0;
    }

    if (pair->value == NULL) {
        #if defined ERROR
          LOG("ERROR: hashtable_check_if_pair_is_good: bad value:  pair->value");
        #endif
        return 0;
    }
    return 1;
}

/* Hash functions */
uint32_t hash (hkey_t key) {
    #if defined DEBUG
    LOG("Get hash from key");
    #endif
    #if defined DEBUG
        LOG("  key: %d", (int)key);
    #endif

    uint32_t hashval = knuth(key);
    return hashval;
}
