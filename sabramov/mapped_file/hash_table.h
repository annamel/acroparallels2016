#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "mapped_file.h"
#include "chunk_manage.h"

#define TABLE_SIZE 1024

typedef struct  hash_node 
{
	int key;
//	void* value;
	struct chunk* value; 
	struct hash_node *next; 
	struct hash_node* prev; 
} hash_node_t; 




#endif 		// HASH_TABLE_H