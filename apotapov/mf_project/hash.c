#include "hash.h"

//=================HASH FUNCTION=====================

hash_key_t hash_func (value_key_t key)
{
  const uint32_t m = 0x5bd1e995;
  const uint32_t seed = 0;
  const int r = 24;

  uint32_t h = seed ^ 4;
  uint32_t k = key;

  k *= m;
  k ^= k >> r;
  k *= m;

  h *= m;
  h ^= k;

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
}
//===================================================

hash_table_t* hash_table_init(const unsigned int size) {
	hash_table_t* table_item = (hash_table_t*)calloc(1, sizeof(hash_table_t));
	if(table_item == NULL) {
		printf("Error in creating hash table structure!\n");
		return NULL;
	}
	table_item -> size = size;
	table_item -> table = (list_element**)calloc(size,sizeof(list_element*)); 
	if((table_item -> table) == NULL) {
		printf("Error in creating hash table array!\n");
		free(table_item);
		return NULL;
	}
    int i = 0;
    for(i = 0; i < size; i++) {
		(table_item -> table)[i] = NULL;
	}
	return table_item;
}

int deinit (hash_table_t* table) {
    int i = 0;
    for(i = 0; i < (table -> size); i++) {
		int j = 0;
		list_element* ptr = (table -> table)[i];
		if(ptr == NULL) {
			continue;
		} else {
			while(ptr -> next) {
				ptr = ptr -> next;
			}
			list_element* buff_ptr = NULL;
			while((ptr -> prev) != NULL) {
				buff_ptr = ptr -> prev;
				free(ptr);
				ptr = buff_ptr;
			}
			free(ptr);
			(table -> table)[i] = NULL;
		}
	}
	return 1;
}

int add_element(hash_table_t* table, value_key_t key, value_t* value) {
	hash_key_t key_hash = hash_func(key); 
	unsigned int place = key_hash % (table -> size);	
	list_element* new_element = (list_element*)calloc(1, sizeof(list_element));
	if(new_element == NULL) {
		printf("Error in creating list element!\n");
		return -1;
	}
	new_element -> next = NULL;
	new_element -> prev = NULL;
	new_element -> key = key;
    new_element -> data = *value;
	list_element* ptr = (table -> table) [place];

	if(ptr == NULL) {
		(table -> table) [place] = new_element;
	} else {
		while(ptr -> next) {
			if(((ptr -> key) == (new_element -> key)) && ((ptr -> data) == (new_element -> data))) {
				return 1;
			}
			ptr = ptr -> next;
		}
		if(((ptr -> key) == (new_element -> key)) && ((ptr -> data) == (new_element -> data))){
			return 1;
		}
		ptr -> next = new_element;
		new_element -> prev = ptr;
	}
	return 0;
}

int find_value(hash_table_t* table, value_key_t key, value_t* value) {
	hash_key_t key_hash = hash_func(key); 
	unsigned int place = key_hash % (table -> size);
	list_element* ptr = (table -> table)[place];
	while(ptr) {
        if(((ptr -> key) == key) && (ptr->data) == *value) {
            return 1;
        }
		ptr = ptr -> next;
	}
    return 0;
}

int remove_element(hash_table_t* table, value_key_t key, value_t* value) {
	hash_key_t key_hash = hash_func(key); 
	unsigned int place = key_hash % (table -> size);
	list_element* ptr = (table -> table)[place];
	if(ptr == NULL) {
		return 0;
	} else {
		while(ptr) {
			if (((ptr -> key) == key) && ((ptr -> data) == value)) {
				if(ptr == (table -> table)[place]) {
					(table -> table)[place] = ptr -> next;
					if(ptr -> next != NULL) {
						ptr -> next -> prev = NULL;
					}
					free(ptr);
					return 1;
				} else { 
					if(ptr -> next != NULL) {
						ptr -> next -> prev = ptr -> prev;
						ptr -> prev -> next = ptr -> next;
					} else {
						ptr -> prev -> next = NULL;
					}
					free(ptr);
					return 1;
				}
			}
		ptr = ptr -> next;
		}
	}
	return 0;
}

int numberOfItemsInIndex(hash_table_t* table, int index) {
	list_element* ptr = (table -> table)[index];
	int count = 0;
	while(ptr) {
		count++;
		ptr = ptr -> next;
	}
	return count;
}

void printElementsInIndex(list_element* ptr, int index) {
	printf("The number of group is: %d:\n",index);
	while(ptr) {
			printf("Key: %u\t\tValue: %llu\n", (ptr -> key), (ptr -> data));
			ptr = ptr -> next;
		}
	printf("The end of group %d:\n\n\n",index);
}

void print_hash_table(hash_table_t* table) {
	list_element* ptr = NULL;
    int i = 0;
    for(i = 0; i < (table -> size); i++) {
		printElementsInIndex((table -> table)[i], i);
	}
}
