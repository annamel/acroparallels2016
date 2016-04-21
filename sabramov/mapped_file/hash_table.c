#include "hash_table.h"

struct hash_node** init_hash_table()
{
	hash_node_t** table = malloc(sizeof(hash_node_t*) * TABLE_SIZE);
	return table;
} 

int deinit_hash_table(hash_node_t** hash_table)
{
	hash_node_t* ptr;
	int i = 0;

	for (i = 0; i < TABLE_SIZE; i++)
	{
		ptr = hash_table[i];

		while (ptr)
		{
			hash_node_t* p  = ptr;
			ptr = ptr->next;
			free(p);
		}
	}


	free(hash_table);
	
	return 0;
}

uint32_t hash_func(uint32_t a)
{
   a = (a+0x7ed55d16) + (a<<12);
   a = (a^0xc761c23c) ^ (a>>19);
   a = (a+0x165667b1) + (a<<5);
   a = (a+0xd3a2646c) ^ (a<<9);
   a = (a+0xfd7046c5) + (a<<3);
   a = (a^0xb55a4f09) ^ (a>>16);
   a = a % TABLE_SIZE;
   return a;
}

/*
int hash_func(int key)
{
	return ((key / 4096) % TABLE_SIZE);
}
*/

hash_node_t* table_lookup(int key, hash_node_t** hash_table) 
{ 
	int h = hash_func(key); 
	
	hash_node_t *p; 
	
	if (hash_table[h] != NULL) 
	{ 
		
		p = hash_table[h]; 
		
		do { 
			if (p -> key == key) 
			{	
				return p;	
			}
			else 
			{
				p = p -> next; 
			}
		} while (p != NULL); 
		
		return NULL; 
	} 

	return NULL;
}

hash_node_t* create_node(int key, chunk_t* value, hash_node_t* next, hash_node_t* prev)
{
	hash_node_t* node = malloc(sizeof(hash_node_t));
	
	if (node == NULL)
		return node;

	node->key = key;
	node->value = value;
	node->next = next;
	node->prev = prev;

	return node; 
}


int insert(int key, chunk_t* value, hash_node_t** hash_table)
{
	int h  = hash_func(key);

	hash_node_t* p;

	if (hash_table[h] != NULL)
	{
		p = hash_table[h];

		do {
			if (p->key == key)
			{
				p->value = value;
				break;
			}
			else
			{
				if (p->next == NULL)
				{
					p->next = create_node(key, value, NULL, p);
					break;
				}	
				else
					p = p->next;
			}	
		} while(p != NULL);
	}
	else
	{
		hash_table[h] = create_node(key, value, NULL, NULL);
	}

	return 0;
}

int delete(int key, hash_node_t** hash_table)
{
	int h = hash_func(key);
	hash_node_t* p;

	if (hash_table[h] == NULL)
		return 0;

	p = hash_table[h];

	if (hash_table[h]->key == key)
	{	
		hash_table[h] = hash_table[h]->next;
		if (hash_table[h])	
			hash_table[h]->prev = NULL;
		free(p);
		return 0;
	}

	do {
		if (p->key == key)
		{
			hash_node_t* ptr = p;
			p->prev->next = p->next;
				
			if (p->next)
				p->next->prev = p->prev;
			free(p);
			return 0;
		}	
		p = p->next;
	} while(p != NULL);

	return 0;
}




