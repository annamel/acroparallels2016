/*

 # hashtable source file
 #
 # Lang:     C
 # Author:   okhlopkov
 # Version:  0.1

 */

#include <stdio.h>
#include <stdint.h>

#include "hashtable.h"
#include "hashfunction.h"
#include "../logger/logger.h"


/* Private methods */

uint32_t hashtable_get_hash_from_key (hashtable_t *hashtable, char *key);
uint32_t hashtable_get_hash_from_pair (hashtable_t *hashtable, hashtable_pair_t *pair);
uint32_t hashtable_check_if_pair_is_good (hashtable_pair_t *pair);

/* Constructors */

hashtable_t *hashtable_create_table (uint32_t size) {
    #if defined DEBUG || defined INFO || defined ERROR
        LOG("Creating new Hashtable");
    #endif
    if (size == 0) {
        #if defined ERROR
           LOG("ERROR: hashtable_create_table: bad argument:  size = 0");
        #endif
        return NULL;
    }
    #if defined DEBUG
    LOG("  size: %u", size);
    #endif
    hashtable_t *hashtable = (hashtable_t *)malloc(sizeof(hashtable_t));
    if (hashtable == NULL) {
        #if defined ERROR
           LOG("ERROR: hashtable_create_table: bad allocation: hashtable");
        #endif
        return NULL;
    }
    //initialising count and size fields.
    hashtable->count = 0;
    hashtable->size = size;
    //allocating memory for data.
    hashtable->arr = (hashtable_pair_t *)calloc(size, sizeof(hashtable_pair_t));
    if (hashtable->arr == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_create_table: bad allocation: hashtable's data");
        #endif
        return NULL;
    }

    //initializing all pairs with NULL.
    hashtable_pair_t *array = hashtable->arr;
    for (uint32_t i = 0; i < size; ++i) {
        array[i].key = NULL;
        array[i].value = NULL;
        array[i].next = NULL;
        array[i].prev = NULL;
    }
    #if defined DEBUG || defined INFO
        LOG("  Table %u was created.", (uint32_t)hashtable);
    #endif
    return hashtable;
}

hashtable_pair_t hashtable_create_pair (char *nkey, char *nvalue) {
    #if defined DEBUG || defined INFO
        LOG("Creating table pair: key -> value");
    #endif

    if (nkey == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_create_pair: bad argument:  nkey");
        #endif
        hashtable_pair_t bad_value = {(char *)EINVAL, (char *)EINVAL};
        return bad_value;
    }

    if (nvalue == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_create_pair: bad argument:  nvalue");
        #endif
        hashtable_pair_t bad_value = {(char *)EINVAL, (char *)EINVAL};
        return bad_value;
    }

    #if defined DEBUG
        LOG("  nkey: %s\n  nvalue: %s", nkey, nvalue);
    #endif

    //allocating memory for copies of key and value.
    char *key_copy = (char *) malloc (sizeof(char) * (strlen(nkey) + 1));
    if (key_copy == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_create_pair: bad allocation:  key_copy");
        #endif
        hashtable_pair_t bad_value = {(char *)1337, (char *)666};
        return bad_value;
    }
    char *value_copy = (char *) malloc (sizeof(char) * (strlen(nvalue) + 1));
    if (value_copy == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_create_pair: bad allocation:  value_copy");
        #endif
        hashtable_pair_t bad_value = {(char *)1337, (char *)666};
        return bad_value;
    }
    //copying key and value.
    key_copy = strcpy(key_copy, nkey);
    value_copy = strcpy(value_copy, nvalue);

    //creating pair.
    hashtable_pair_t a = {.key = key_copy, .value = value_copy, .next = NULL, .prev = NULL};
    #if defined DEBUG
        LOG("  pair created:\n  key:%s\n  value:%s", key_copy, value_copy);
    #endif
    return a;
}

/* Destructor */

uint32_t hashtable_delete_table (hashtable_t *hashtable) {
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
      LOG("ERROR: hashtable_delete_table: bad argument:  hashtable->arr");
    #endif
    return EINVAL;
    }

    hashtable_pair_t *array = hashtable->arr;

    //freeing all keys-value pairs.
    uint32_t size = hashtable_size(hashtable);
    for (int i = 0; i < size; i++) {
        if ((array[i]).key != NULL) {
            free(array[i].value);
            free(array[i].key);
            (array[i]).key = NULL;
            (array[i]).value = NULL;
            hashtable_pair_t *next_pair = (array[i]).next;
            while (next_pair != NULL) {
                free(next_pair->value);
                free(next_pair->key);
                next_pair->key = NULL;
                next_pair->value = NULL;
                next_pair->prev = NULL;
                next_pair = next_pair->next;
            }
            (array[i]).next = NULL;
            (array[i]).prev = NULL;
        }
    }

    free (array);
    hashtable->arr = NULL;
    free (hashtable);


    #if defined DEBUG || defined INFO || defined ERROR
    LOG("Hashtable was successfully destructed.");
    #endif
    return 0;
}

/* Public properties */

uint32_t hashtable_is_empty (hashtable_t *hashtable) {
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_is_empty: bad argument:  hashtable");
        #endif
        return EINVAL;
    }
    #if defined DEBUG
        LOG(" _is_empty for: %u", (uint32_t)hashtable);
    #endif
    return hashtable->count ? 0 : 1;
}

uint32_t hashtable_count(hashtable_t *hashtable) {
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_count: bad argument:  hashtable");
        #endif
        return EINVAL;
    }
    #if defined DEBUG
        LOG(" -count for: %u", (uint32_t)hashtable);
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
        LOG(" -size for: %u", (uint32_t)hashtable);
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
          LOG("ERROR: hashtable_add_pair: bad allocation:  array");
        #endif
        return EINVAL;
    }
    if (!hashtable_check_if_pair_is_good (&pair)) return EINVAL;
    #if defined DEBUG || defined INFO
    LOG("Add pair to: %u", (uint32_t)hashtable);
    #endif

    //assigning to new pointer in order to write less code.
    hashtable_pair_t *array = hashtable->arr;

    //i is first found hash of the key.
    uint32_t i = hashtable_get_hash_from_pair (hashtable, &pair);

    /* // PART FOR DIFFERENT HASHTABLE IMPLEMENTATION
    //looking for first empty place to put new pair.
    uint32_t hashtbl_size = hashtable_size (hashtable);
    while ((array[i].key != NULL) && (strcmp((array[i]).key, pair.key) != 0)) {
        i += DEFAULT_STEPPING;
        if (i == hashtbl_size) i = 0;
    }

    //placing pair into array, incrementing counter.
    if (array[i].key == NULL) {
        array[i].key = pair.key;
        array[i].value = pair.value;
        hashtable->count ++;
    }
    */

    if (array[i].key == NULL) {
        array[i].key = pair.key;
        array[i].value = pair.value;
        array[i].next = NULL;
        array[i].prev = NULL;
    } else {
        hashtable_pair_t *current_pair = array[i].next;
        hashtable_pair_t *prev_pair = hashtable->arr + i;
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

uint32_t hashtable_add_pair_by_key_value (hashtable_t *hashtable, char *key, char *value) {
    #if defined DEBUG || defined INFO
        LOG("Add by key value");
    #endif
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_add_pair_by_key_value: bad argument:  hashtable");
        #endif
        return EINVAL;
    }
    if (key == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_add_pair_by_key_value: bad argument:  key");
        #endif
        return EINVAL;
    }
    if (value == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_add_pair_by_key_value: bad argument:  value");
        #endif
        return EINVAL;
    }
    #if defined DEBUG
        LOG("  to hashtable: %u\n  with key: %s\n  value: %s",
                              (uint32_t)hashtable, key, value);
    #endif
    return hashtable_add_pair (hashtable, hashtable_create_pair (key, value));
}

// Getters
char *hashtable_get_value_by_key (hashtable_t *hashtable, char *key) {
    #if defined DEBUG || defined INFO
    LOG("Get value by key");
    #endif
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_get_value_by_key: bad argument:  hashtable");
        #endif
        return NULL;
    }
    if (key == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_get_value_by_key: bad argument:  key");
        #endif
        return NULL;
    }
    if (hashtable->arr == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_get_value_by_key: bad argument:  hashtable->arr");
        #endif
        return NULL;
    }
    #if defined DEBUG
        LOG("  hashtable: %u\n  key: %s", (uint32_t)hashtable, key);
    #endif

    //getting first hash from key and saving it for future use (in order not to count hash again).
    uint32_t i = hashtable_get_hash_from_key (hashtable, key);

    //looking for value.
    int not_found = 1;
    hashtable_pair_t *current_pair = hashtable->arr + i;

    while (current_pair != NULL && not_found) {
        if (strcmp(current_pair->key, key) == 0)
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

	/* Creating copy of Value to return. */

	//asserting value in array is not NULL.
    if (current_pair->value == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_add_pair: bad value in hashtable->arr\n  at index: %u",
                                (uint32_t)i);
        #endif
        return NULL;
    }

	//allocating memory for new value instance. Size is incremented because of '\0' character.
	char *value_copy = (char *) malloc (sizeof(char) * (strlen(current_pair->value) + 1));
    if (value_copy == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_add_pair: bad allocation:  value_copy");
        #endif
        return NULL;
    }

	//copying value.
	value_copy = strcpy(value_copy, (current_pair->value));

    #if defined DEBUG
        LOG("  from hashtable: %u\n  with key: %s\n  value: %s",
                          (uint32_t)hashtable, key, value_copy);
    #endif
    return value_copy;
}

hashtable_pair_t *hashtable_get_pair_by_key (hashtable_t *hashtable, char *key) {
  #if defined DEBUG || defined INFO
    LOG("Get pair by key");
  #endif
  if (hashtable == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_pair_by_key: bad argument:  hashtable");
    #endif
    return NULL;
  }
  if (key == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_pair_by_key: bad argument:  key");
    #endif
    return NULL;
  }
  //retreiving value.
  char *nvalue = hashtable_get_value_by_key (hashtable, key);
  if (nvalue == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_pair_by_key: can't get value by key");
    #endif
    return NULL;
  }
  #if defined DEBUG
    LOG("  hashtable: %u\n  key: %s", (uint32_t)hashtable, key);
  #endif

	//copying key.
  char *key_copy = (char *) malloc (sizeof(char) * (strlen(key) + 1));
  key_copy = strcpy(key_copy, key);

  //creating pair to return.
  hashtable_pair_t *pair = (hashtable_pair_t *)malloc(sizeof(hashtable_pair_t));
  if (pair == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_pair_by_key: bad allocation:  hashtable");
    #endif
    return NULL;
  }

  pair->key = key_copy;
  pair->value = nvalue;
  #if defined DEBUG
    LOG("  pair was founded at key: %s\n  value: %s", key_copy, nvalue);
  #endif
  return pair;

}

// Deletion
uint32_t hashtable_delete_pair_by_key (hashtable_t *hashtable, char *key) {
    #if defined DEBUG || defined INFO
     LOG("Delete pair by key");
    #endif
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_delete_pair_by_key: bad argument:  hashtable");
        #endif
        return EINVAL;
    }
    if (key == NULL) {
        #if defined ERROR
           LOG("ERROR: hashtable_delete_pair_by_key: bad argument:  key");
        #endif
        return EINVAL;
    }
    if (hashtable->arr == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_delete_pair_by_key: bad argument:  hashtable->arr");
        #endif
        return EINVAL;
    }
    #if defined DEBUG
        LOG("  hashtable: %u\n  key: %s\n", (uint32_t)hashtable, key);
    #endif


    uint32_t i = hashtable_get_hash_from_key (hashtable, key);

    int not_found = 1;
    hashtable_pair_t *current_pair = hashtable->arr + i;

    while (current_pair != NULL && not_found) {
        if (strcmp(current_pair->key, key) == 0)
            not_found = 0;
        else
            current_pair = current_pair->next;
    }

    if (not_found) {
        #if defined ERROR
            LOG("ERROR: hashtable_delete_pair_by_key: pair not found");
        #endif
        return 1;
    }

	//freeing allocated memory and deleting.
    free(current_pair->value);
    free(current_pair->key);
    if (current_pair->prev != NULL) {
        current_pair->prev->next = current_pair->next;
        current_pair->prev = NULL;
    }
    current_pair->key = NULL;
    current_pair->value = NULL;

	hashtable->count --;
    #if defined DEBUG
        LOG("  pair was successufully deleted.");
    #endif
	return 0;
}

/* Private methods */

uint32_t hashtable_check_if_pair_is_good (hashtable_pair_t *pair) {
    #if defined DEBUG
        LOG(" -checking if pair is good");
    #endif
    if (pair == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_check_if_pair_is_good: bad argument:  pair");
        #endif
        return 0;
    }
    if (pair->key == NULL) {
        #if defined ERROR
          LOG("ERROR: hashtable_check_if_pair_is_good: bad argument:  pair->key");
        #endif
        return 0;
    }
    if (pair->value == NULL) {
        #if defined ERROR
          LOG("ERROR: hashtable_check_if_pair_is_good: bad argument:  pair->value");
        #endif
        return 0;
    }
    return 1;
}

/* Hash functions */
uint32_t hashtable_get_hash_from_pair (hashtable_t *hashtable, hashtable_pair_t *pair) {
    #if defined DEBUG || defined INFO
        LOG("Get hash from pair");
    #endif
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_get_hash_from_pair: bad argument:  hashtable");
        #endif
        return EINVAL;
    }
    if (!hashtable_check_if_pair_is_good (pair)) {
        #if defined ERROR
            LOG("ERROR: hashtable_get_hash_from_pair: bad argument:  pair");
        #endif
        return EINVAL;
    }
    #if defined DEBUG
        LOG("  from hashtable: %u", (uint32_t)hashtable);
    #endif
    return hashtable_get_hash_from_key (hashtable, pair->key);
}

uint32_t hashtable_get_hash_from_key (hashtable_t *hashtable, char *key) {
    #if defined DEBUG
    LOG("Get hash from key");
    #endif
    if (hashtable == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_get_hash_from_key: bad argument:  hashtable");
        #endif
        return EINVAL;
    }
    if (key == NULL) {
        #if defined ERROR
            LOG("ERROR: hashtable_get_hash_from_key: bad argument:  key");
        #endif
        return EINVAL;
    }
    #if defined DEBUG
        LOG("  hashtable: %u\n  key: %s", (uint32_t)hashtable, key);
    #endif
    uint32_t hashval = hash_knuth(key);
    hashval %= hashtable_size(hashtable);

    return hashval;
}
