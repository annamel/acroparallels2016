/*

 # hashtable source file
 #
 # Lang:     C
 # Author:   okhlopkov
 # Version:  0.1

 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "hashtable.h"
#include "hashfunction.h"
#include "logger/logger.h"


/* Private methods */

uint32_t hashtable_get_hash_from_key (hashtable_t *hashtable, char *key);
uint32_t hashtable_get_hash_from_pair (hashtable_t *hashtable, hashtable_pair_t *pair);
uint32_t hashtable_check_if_pair_is_good (hashtable_pair_t *pair);

/* Constructors */

hashtable_t *hashtable_create_table (uint32_t size) {
  #if defined DEBUG || defined INFO || defined ERROR
    logger = logger_init();
    LOG("Initializing logger");
  #endif
  #if defined DEBUG || defined INFO
    LOG("Creating new table");
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
    hashtable_pair_t bad_value = {(char *)BAD_ARGUMENT, (char *)BAD_ARGUMENT};
    return bad_value;
  }

  if (nvalue == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_create_pair: bad argument:  nvalue");
    #endif
    hashtable_pair_t bad_value = {(char *)BAD_ARGUMENT, (char *)BAD_ARGUMENT};
    return bad_value;
  }

  #if defined DEBUG
    LOG("  nkey: %s\n  nvalue: %s", nkey, nvalue);
  #endif

	//allocating memory for copies of key and value.
	char *key_copy = (char *) malloc (sizeof(char) * (strlen(nkey) + 1));
	char *value_copy = (char *) malloc (sizeof(char) * (strlen(nvalue) + 1));
  if (key_copy == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_create_pair: bad allocation:  key_copy");
    #endif
    hashtable_pair_t bad_value = {(char *)1337, (char *)666};
    return bad_value;
  }
  if (value_copy == NULL) {
    #if defined ERROR
      LOG("hashtable_create_pair: bad allocation:  value_copy");
    #endif
    hashtable_pair_t bad_value = {(char *)1337, (char *)666};
    return bad_value;
  }
	//copying key and value.
	key_copy = strcpy(key_copy, nkey);
	value_copy = strcpy(value_copy, nvalue);

	//creating pair.
	hashtable_pair_t a = {key_copy, value_copy};
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
    return BAD_ARGUMENT;
  }
  if (hashtable->arr == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_delete_table: bad argument:  hashtable->arr");
    #endif
    return BAD_ARGUMENT;
  }

  hashtable_pair_t *array = hashtable->arr;

  //freeing all keys-value pairs.
  uint32_t size = hashtable_size(hashtable);
  for (int i = 0; i < size; i++) {
      if ((array[i]).key != NULL) {
//          free ((array[i]).key);
//          free ((array[i]).value);
          (array[i]).key = NULL;
          (array[i]).value = NULL;
      }
  }

  free (array);
  hashtable->arr = NULL;
  free (hashtable);


  #if defined DEBUG || defined INFO || defined ERROR
    LOG("Hashtable was successfully destructed.");
    logger_deinit(logger);
  #endif
	return 0;
}

/* Public properties */

uint32_t hashtable_is_empty (hashtable_t *hashtable) {
  if (hashtable == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_is_empty: bad argument:  hashtable");
    #endif
    return BAD_ARGUMENT;
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
    return BAD_ARGUMENT;
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
    return BAD_ARGUMENT;
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
    return BAD_ARGUMENT;
  }
  if (hashtable->arr == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_add_pair: bad allocation:  array");
    #endif
    return BAD_ARGUMENT;
  }
  if (!hashtable_check_if_pair_is_good (&pair)) return BAD_ARGUMENT;
  #if defined DEBUG || defined INFO
    LOG("Add pair to: %u", (uint32_t)hashtable);
  #endif

	//assigning to new pointer in order to write less code.
	hashtable_pair_t *array = hashtable->arr;
	//check whether memory is allocated and there is place for one more pair.
  if (hashtable_count (hashtable) >= hashtable_size (hashtable)) {
    #if defined ERROR
      LOG("ERROR: hashtable_add_pair: size exceeded");
    #endif
    return SIZE_EXCEEDED;
  }

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

  if (array[i].key == NULL) hashtable->count ++;
  array[i].key = pair.key;
  array[i].value = pair.value;

	//returning hash id.
  #if defined DEBUG
    LOG("  pair added at  %u\n  to table: %u", (uint32_t)i, (uint32_t)hashtable);
  #endif
	return i;
}

uint32_t hashtable_add_pair_by_key_value (hashtable_t *hashtable, char *key, char *value) {
  #if defined DEBUG || defined INFO
    LOG("Add by key value");
  #endif
  if (hashtable == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_add_pair_by_key_value: bad argument:  hashtable");
    #endif
    return BAD_ARGUMENT;
  }
  if (key == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_add_pair_by_key_value: bad argument:  key");
    #endif
    return BAD_ARGUMENT;
  }
  if (value == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_add_pair_by_key_value: bad argument:  value");
    #endif
    return BAD_ARGUMENT;
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

	//using array pointer in order to write less code.
	hashtable_pair_t *array = hashtable->arr;

	//getting first hash from key and saving it for future use (in order not to count hash again).
	uint32_t i = hashtable_get_hash_from_key (hashtable, key);
	uint32_t hash = i;

	//getting size of hashtable.
	uint32_t hashtbl_size = hashtable_size (hashtable);

	//var to understand when we cycled through all table.
	uint32_t runned_through_cycle = 0;

	//looking for value.
	while ((array[i].key != NULL) && (strcmp((array[i]).key, key) != 0) && runned_through_cycle == 0) {
		i += DEFAULT_STEPPING;
		if (i == hashtbl_size) i = 0;
		if (i == hash) runned_through_cycle = 1;
	}

	//returning NULL in cases of not found.
  if (runned_through_cycle == 1 || array[i].key == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_value_by_key: element not found");
    #endif
    return NULL;
  }


	/* Creating copy of Value to return. */

	//asserting value in array is not NULL.
  if (array[i].value == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_add_pair: bad value in hashtable->arr\n  at index: %u",
                            (uint32_t)i);
    #endif
    return NULL;
  }

	//allocating memory for new value instance. Size is incremented because of '\0' character.
	char *value_copy = (char *) malloc (sizeof(char) * (strlen((array[i]).value) + 1));
  if (value_copy == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_add_pair: bad allocation:  value_copy");
    #endif
    return NULL;
  }

	//copying value.
	value_copy = strcpy(value_copy, (array[i]).value);

  #if defined DEBUG
    LOG("  from hashtable: %u\n  with key: %s\n  value: %s",
                              (uint32_t)hashtable, key, value_copy);
  #endif
	return value_copy;
}

char *hashtable_get_value_by_id (hashtable_t *hashtable, uint32_t hash_id) {
  #if defined DEBUG || defined INFO
    LOG("Get value by id");
  #endif
  if (hashtable == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_value_by_id: bad argument:  hashtable");
    #endif
    return NULL;
  }
  if (hash_id >= hashtable_size(hashtable)) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_value_by_id: bad argument:  hash_id is to big");
    #endif
    return NULL;
  }
  //creating array pointer in order to write less code.
  hashtable_pair_t *array = hashtable->arr;
  if (array == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_value_by_id: bad argument:  array");
    #endif
    return NULL;
  }
  if (array[hash_id].key == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_value_by_id: bad argument:  array[hash_id].key");
    #endif
    return NULL;
  }
  if (array[hash_id].value == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_value_by_id: bad argument:  array[hash_id].value");
    #endif
    return NULL;
  }

  #if defined DEBUG
    LOG("  hashtable: %u\n  hash_id: %u", (uint32_t)hashtable, hash_id);
  #endif

	/* Creating copy of Value. */

	//allocating memory.
	char *value_copy = (char *) malloc (sizeof(char) * (strlen((array[hash_id]).value) + 1));
  if (value_copy == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_value_by_id: bad allocation:  value_copy");
    #endif
    return NULL;
  }

	//copying value.
	value_copy = strcpy(value_copy, (array[hash_id]).value);

  #if defined DEBUG
    LOG("  pair was founded with value: %s", array[hash_id].value);
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
  if (hashtable == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_pair_by_key: bad allocation:  key_copy");
    #endif
    return NULL;
  }
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

hashtable_pair_t *hashtable_get_pair_by_id (hashtable_t *hashtable, uint32_t hash_id) {
  #if defined DEBUG || defined INFO
    LOG("Get pair by id");
  #endif
  if (hashtable == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_pair_by_id: bad argument:  hashtable");
    #endif
    return NULL;
  }
  if (hashtable->arr == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_delete_pair_by_id: bad argument:  hashtable->arr");
    #endif
    return NULL;
  }

  #if defined DEBUG
    LOG("  hashtable: %u\n  hash_id: %u", (uint32_t)hashtable, hash_id);
  #endif
	//using array pointer in order to write less code.
	hashtable_pair_t *array = hashtable->arr;

	//getting value.
	char *nvalue = hashtable_get_value_by_id (hashtable, hash_id);
	if (nvalue == NULL) return NULL;

	//getting key.
	char *key = (array[hash_id]).key;
	if (key == NULL) return NULL;

	//copying key instance.
	char *key_copy = (char *) malloc (sizeof(char) * (strlen(key) + 1));
  if (key_copy == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_delete_pair_by_id: bad allocation:  key_copy");
    #endif
    return NULL;
  }
	key_copy = strcpy(key_copy, key);

	//creating pair to return.
	hashtable_pair_t *pair = (hashtable_pair_t *)malloc(sizeof(hashtable_pair_t));
  if (pair == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_pair_by_id: bad allocation:  pair");
    #endif
    return NULL;
  }
	pair->key = key_copy;
	pair->value = nvalue;
  #if defined DEBUG
    LOG("  pair was founded at key: %s\n  with value: %s", key_copy, nvalue);
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
    return BAD_ARGUMENT;
  }
  if (key == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_delete_pair_by_key: bad argument:  key");
    #endif
    return BAD_ARGUMENT;
  }
  if (hashtable->arr == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_delete_pair_by_key: bad argument:  hashtable->arr");
    #endif
    return BAD_ARGUMENT;
  }
  #if defined DEBUG
    LOG("  hashtable: %u\n  key: %s\n", (uint32_t)hashtable, key);
  #endif
	//using array pointer in order to write less code.
	hashtable_pair_t *array = hashtable->arr;

	/* Looking for pair in the same manner as when adding */
	uint32_t i = hashtable_get_hash_from_key (hashtable, key);
	uint32_t hash = i;
	uint32_t hashtbl_size = hashtable_size (hashtable);
	uint32_t runned_through_cycle = 0;

	while ((array[i].key != NULL) && (strcmp((array[i]).key, key) != 0) && runned_through_cycle == 0) {
		i += DEFAULT_STEPPING;
		if (i == hashtbl_size) i = 0;
		if (i == hash) runned_through_cycle = 1;
	}

	//if no pair found, return 1
  if (runned_through_cycle == 1 || array[i].key == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_delete_pair_by_key: pair not found");
    #endif
    return 1;
  }

	//freeing allocated memory and deleting.
	free(array[i].key);
	free(array[i].value);
	array[i].key = NULL;
	array[i].value = NULL;
	hashtable->count --;
  #if defined DEBUG
    LOG("  pair was successufully deleted.");
  #endif
	return 0;
}

uint32_t hashtable_delete_pair_by_id (hashtable_t *hashtable, uint32_t hash_id) {
  #if defined DEBUG || defined INFO
    LOG("Delete pair by id");
  #endif
  if (hashtable == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_delete_pair_by_id: bad argument:  hashtable");
    #endif
    return BAD_ARGUMENT;
  }
  if (hash_id >= hashtable_size (hashtable)) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_hash_from_pair: bad argument:  hash_id is too large");
    #endif
    return BAD_ARGUMENT;
  }
  if (hashtable->arr == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_hash_from_pair: bad argument:  hashtable->arr");
    #endif
    return BAD_ARGUMENT;
  }
  #if defined DEBUG
    LOG("  hashtable: %u\n  hash_id: %u", (uint32_t)hashtable, hash_id);
  #endif
	//using array pointer in order to write less code.
	hashtable_pair_t *array = hashtable->arr;

	if (array[hash_id].key == NULL) return 0;

	//freeing allocated memory and deleting.
	free(array[hash_id].key);
	free(array[hash_id].value);
	array[hash_id].key = NULL;
	array[hash_id].value = NULL;
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
    return BAD_ARGUMENT;
  }
  if (!hashtable_check_if_pair_is_good (pair)) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_hash_from_pair: bad argument:  pair");
    #endif
    return BAD_ARGUMENT;
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
    return BAD_ARGUMENT;
  }
  if (key == NULL) {
    #if defined ERROR
      LOG("ERROR: hashtable_get_hash_from_key: bad argument:  key");
    #endif
    return BAD_ARGUMENT;
  }
  #if defined DEBUG
    LOG("  hashtable: %u\n  key: %s", (uint32_t)hashtable, key);
  #endif
	uint32_t hashval = hash_knuth(key);
	hashval %= hashtable_size(hashtable);

	return hashval;
}
