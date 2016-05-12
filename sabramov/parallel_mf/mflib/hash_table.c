#include "chunk_manage.h"
#include "mapped_file.h"
#include <stdlib.h>	
#include <string.h>
#include <stdint.h>

struct chunk** init_hash_table()
{
	chunk_t** htable = malloc(sizeof(chunk_t*) * TABLE_SIZE);
	memset(htable, 0, (sizeof(chunk_t*) * TABLE_SIZE)/sizeof(char));
	return htable;
} 

void deinit_hash_table(chunk_t** hash_table)
{
	free(hash_table);
}


int fast_lookup(long long key, chunk_t* chunk, file_handle_t* fh)
{
	chunk_t** hash_table = fh->hash_table;
	int h = hash_func(key);
	chunk_t* p = NULL;
		
	for (p = hash_table[h]; p != NULL; p = p->next)
		if (p -> pg_round_down_off == key)		
			return (p->size == chunk->size);			
	
	return 0;
}

/*
int hash_func(long long a)
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
*/

int hash_func(long long key)
{
	return ((key / 4096) % TABLE_SIZE);
}


chunk_t* table_lookup(long long key, file_handle_t* fh) 
{ 
	chunk_t** hash_table = fh->hash_table;
	int h = hash_func(key);
	chunk_t* p = NULL; 
 
	for (p = hash_table[h]; p != NULL; p = p->next)
		if (p -> pg_round_down_off == key) 
			return p;	

	return NULL;
}

int push_chunk(long long key, chunk_t** chunk_ptr, file_handle_t** fh)
{
	chunk_t** hash_table = (*fh)->hash_table;
	int h  = hash_func(key);


	if (hash_table[h] != NULL)
	{
		chunk_t* p = hash_table[h];

		do {
			
			if (p->pg_round_down_off == key)
			{				
				chunk_t* ch = *chunk_ptr;
				*chunk_ptr = (*chunk_ptr)->next;
				
				if (!p->prev)
				{
					ch->next = p->next;
					ch->prev = p->prev;
					hash_table[h] = ch;
				}
				else 
				{
					ch->next = p->next;
					ch->prev = p->prev;
				}
				
				if (ch->prev)
					ch->prev->next = ch;
				
				if (ch->next)
					ch->next->prev = ch;

				if (!p->ref_count)			
				{	
					p->next = NULL;
					int unmap = munmap(p->addr, p->size);
					(*fh)->cur_mem_usage -= p->size;
					p->prev = (*fh)->free_chunks_tail;
					(*fh)->free_chunks_tail->next = p;
					(*fh)->free_chunks_tail = p;
					if (!(*chunk_ptr))
						*chunk_ptr = p;
				}
				else
				{
					if (!(*chunk_ptr))
						(*fh)->free_chunks_tail = NULL;
				}
				
				break;
			} 
			else
			{				
				if (!p->next)
				{	
					p->next = *chunk_ptr;
					*chunk_ptr = (*chunk_ptr)->next;
					p->next->prev = p;
					p->next->next = NULL;

					if (!(*chunk_ptr))
						(*fh)->free_chunks_tail = NULL;

					break;
				}
				else
					p = p->next;
			}
		} while (p != NULL);		
	}
	else
	{
		hash_table[h] = *chunk_ptr;
		(*chunk_ptr) = (*chunk_ptr)->next;

		if ((*chunk_ptr) == NULL)
			(*fh)->free_chunks_tail = NULL;

		if ((*chunk_ptr))
			(*chunk_ptr)->prev = NULL;

		hash_table[h]->next = NULL;
		hash_table[h]->prev = NULL;
	}	

	return 0;
}

int hash_table_unmap(chunk_t** hash_table)
{
	chunk_t* ptr = NULL;
	int i = 0;

	for (i = 0; i < TABLE_SIZE; i++)
	{
		for (ptr = hash_table[i]; ptr != NULL; ptr = ptr->next)
			munmap(ptr->addr, ptr->size);			
	}

	return 0;
}


