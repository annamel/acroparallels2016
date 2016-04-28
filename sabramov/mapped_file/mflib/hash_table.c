#include "chunk_manage.h"
#include "mapped_file.h"

#define TABLE_SIZE 1024


struct chunk** init_hash_table()
{
	chunk_t** htable = malloc(sizeof(chunk_t*) * TABLE_SIZE);
	memset(htable, 0, (sizeof(chunk_t*) * TABLE_SIZE)/sizeof(char));

	return htable;
} 

inline void deinit_hash_table(chunk_t** hash_table)
{
	free(hash_table);
}

int fast_lookup(int key, chunk_t* chunk, file_handle_t* fh)
{
	chunk_t** hash_table = fh->hash_table;

	int h = hash_func(key);

	if (hash_table[h] != NULL) 
	{ 
		chunk_t* p = hash_table[h]; 
		
		do { 
			
			if (p -> pg_multiple_offset == key) 
			{	
				if (p->size == chunk->size)
					return 1;	
				else
					return 0;
			}
			else 
				p = p -> next; 
		
		} while (p != NULL); 
		
		return 0; 
	} 

	return 0;	
}

/*
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
*/

int hash_func(int key)
{
	return ((key / 4096) % TABLE_SIZE);
}


chunk_t* table_lookup(int key, file_handle_t* fh) 
{ 
	chunk_t** hash_table = fh->hash_table;
	// chunk_t* p; 

	int h = hash_func(key); 
	
	if (hash_table[h] != NULL) 
	{ 
		chunk_t* p = hash_table[h]; 
		
		do { 
			
			if (p -> pg_multiple_offset == key) 
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

int push_chunk(int key, chunk_t** chunk_ptr, file_handle_t** fh)
{
	chunk_t** hash_table = (*fh)->hash_table;

	int h  = hash_func(key);

	if (hash_table[h] != NULL)
	{
		chunk_t* p = hash_table[h];

		do {
			
			if (p->pg_multiple_offset == key)
			{				
				chunk_t* ch = *chunk_ptr;

				*chunk_ptr = (*chunk_ptr)->next;
				
				if (p->prev == NULL)
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
				if (p->next == NULL)
				{	
					p->next = *chunk_ptr;			// FIXME: need to test
					
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

int pull_chunk(int key, file_handle_t* fh)
{
	chunk_t** hash_table = fh->hash_table;
	chunk_t* p;
	
	int h = hash_func(key);	

	if (hash_table[h] == NULL)
		return 0;

	p = hash_table[h];

	do {

		if (p->pg_multiple_offset == key)
		{
			if (p->prev)
				p->prev->next = p->next;

			if (p->next)
				p->next->prev = p->prev;

			if (!p->ref_count)
			{
				p->next = fh->free_chunks->next;
				p->next->prev = p;
				p->prev = NULL;
				fh->free_chunks = p;
			} 
			
			break;
		}
		else
			p = p->next;
	
	} while (p != NULL);

	return 0;
}

int hash_table_unmap(chunk_t** hash_table)
{
	int i = 0;

	for (i = 0; i < TABLE_SIZE; i++)
	{
		chunk_t* ptr = hash_table[i];

		while (ptr)
		{
			munmap(ptr->addr, ptr->size);
			ptr = ptr->next;
		}
	}

	return 0;
}

