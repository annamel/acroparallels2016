#include "common_types.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "hash_table.h"

#define DEFAULT_HASH_TABLE_SIZE 1024

//=================HASH FUNCTION======================

hash_key_t hash_func (value_key_t key)
{

  const uint32_t m = 0x5bd1e995;
  const uint32_t seed = 0;
  const int r = 24;

  uint32_t h = seed ^ 4;
  uint32_t k = (uint32_t)key;

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


hash_table_t* hash_table_init(unsigned int size) {
 
  if(size == 0) {
    size = DEFAULT_HASH_TABLE_SIZE;
  }

  hash_table_t* buf = (hash_table_t*)calloc(1, sizeof(hash_table_t));
  if(buf == NULL) {
    printf("Initialization of hash table failed!\n");
    return NULL;
  }
  int i = 0;
  buf -> table = (list_element**)calloc(size, sizeof(list_element*));
  buf -> size = size;
  for(i = 0; i < (buf -> size); i++) {
    (buf -> table)[i] = NULL;
  }
  buf -> is_initialized = 1;
  return buf;
}

int hash_table_deinit(hash_table_t* h_table) {
  int i = 0;
  for(i = 0; i < (h_table -> size); i++) {
    int j = 0;
    list_element* ptr = (h_table -> table)[i];
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
      (h_table -> table)[i] = NULL;
    }
  }
  free(h_table -> table);
  free(h_table);
  return 0;
}

int add_element(chunk_t* chunk) {
  hash_key_t key_hash = hash_func(chunk -> index);
  unsigned int place = key_hash % (chunk -> ch_pool -> h_table -> size);
  list_element* elem = (list_element*)calloc(1, sizeof(list_element));
  elem -> next = NULL;
  elem -> prev = NULL;
  elem -> data = chunk;
  list_element* ptr = (chunk -> ch_pool -> h_table -> table) [place];
  if(ptr == NULL) {
    (chunk -> ch_pool -> h_table -> table) [place] = elem;
  } else {
    while(ptr -> next) {
      if(((ptr -> data -> index) == (elem -> data -> index)) && ((ptr -> data -> length) == (elem -> data -> length))) {
        return 1;
      }
      ptr = ptr -> next;
    }
    ptr -> next = elem;
    elem -> prev = ptr;
  }
  return 0;
}

int remove_element(hash_table_t* h_table, off_t index, off_t length) {
  hash_key_t key_hash = hash_func(index);
  unsigned int place = key_hash % (h_table -> size);
  list_element* ptr = (h_table -> table)[place];
  if(ptr == NULL) {
    return 1;
  } else {
    while(ptr) {
      if (((ptr -> data -> index) == index) && ((ptr -> data -> length) == length)) {
        if(ptr == (h_table -> table)[place]) {
          (h_table -> table)[place] = ptr -> next;
          if(ptr -> next != NULL) {
            ptr -> next -> prev = NULL;
          }
          free(ptr);
          return 0;
        } else {
          if(ptr -> next != NULL) {
            ptr -> next -> prev = ptr -> prev;
            ptr -> prev -> next = ptr -> next;
          } else {
            ptr -> prev -> next = NULL;
          }
          free(ptr);
          return 0;
        }
      }
    ptr = ptr -> next;
    }
  }
  return 1;
}

int find_value(hash_table_t* h_table, off_t index, off_t length) {
  hash_key_t key_hash = hash_func(index);
  unsigned int place = key_hash % (h_table -> size);
  list_element* ptr = (h_table -> table)[place];
  while(ptr) {
    if(((ptr -> data -> index) == index) && ((ptr -> data -> length) == length)) {
      ptr -> data-> ref_counter += 1;
      return 1;
    }
    ptr = ptr -> next;
  }
  return 0;
}

chunk_t* take_value_ptr(hash_table_t* h_table, off_t index, off_t length) {
  hash_key_t key_hash = hash_func(index);
  unsigned int place = key_hash % (h_table -> size);
  list_element* ptr = (h_table -> table)[place];
  while(ptr) {
    if(((ptr -> data -> index) == index) && ((ptr -> data -> length) == length)) {
      return ptr -> data;
    }
    ptr = ptr -> next;
  }
  return NULL;
}

chunk_t* find_by_index(hash_table_t* h_table, off_t index) {
  hash_key_t key_hash = hash_func(index);
  unsigned int place = key_hash % (h_table -> size);
  list_element* ptr = (h_table -> table)[place];
  while(ptr) {
    if((ptr -> data -> index) == index) {
      return ptr -> data;
    }
    ptr = ptr -> next;
  }
  return NULL;
}

chunk_t* find_in_range(hash_table_t* h_table, off_t offset, size_t size) {
  int i = 0;
  list_element* ptr = NULL;
  off_t offset_chunk =  0;
  off_t size_chunk = 0;
  for(i = 0; i < (h_table -> size); i++) {
    ptr = (h_table -> table)[i];
    while(ptr) {
      offset_chunk = (ptr -> data -> index) * get_chunk_size(1);
      size_chunk = (ptr -> data -> length) * get_chunk_size(1);
      if((offset_chunk <= offset) && ((offset_chunk + size_chunk) >= (offset + size))) {
        return ptr -> data;
      }
      ptr = ptr -> next;
    }
  }
  return NULL;
}

