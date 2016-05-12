#include "hashtable.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void hashtable_init(hashtable_t* h, size_t size, hashfunction hash)
{
	assert(h);
	assert(size > 0);
	
	h->size = size;
	h->hash = hash;

	h->elements = malloc(size * sizeof(_list_t*));
	assert(h->elements);
	memset(h->elements, 0, size * sizeof(_list_t*));
}

void hashtable_destroy(hashtable_t* h, destroyfunction destroy, void* data)
{
	assert(h);

	size_t i;
	for (i = 0; i < h->size; i++)
	{
		_list_t* l = h->elements[i];
		while (l)
		{
			if (destroy)
				destroy(l->key, l->data, data);

			_list_t* next = l->next;
			free(l);
			l = next;
		}
	}
	free(h->elements);
	h->size = 0;
}

int hashtable_add(hashtable_t* h, uint64_t key, void* element)
{
	assert(h);
	assert(element);
	
	uint64_t hash = h->hash(key);
	unsigned elem = hash % h->size;
	
	_list_t* l = h->elements[elem];
	
	if (!l)
	{
		h->elements[elem] = malloc(sizeof (_list_t));
		l = h->elements[elem];
	}
	else
	{
		while (l->next)
		{
			if (l->key == key)
				return 0;
			l = l->next;
		}
		
		if (l->key == key)
			return 0;
	
		l->next = malloc(sizeof (_list_t));
		l = l->next;
	}
	
	l->next = NULL;
	l->key = key;
	l->data = element;
	
	return 1;
}

void* hashtable_get(hashtable_t* h, uint64_t key)
{
	assert(h);
	
	uint64_t hash = h->hash(key);
	unsigned elem = hash % h->size;
	_list_t* l = h->elements[elem];
	while (l)
	{
		if (l->key == key)
			return l->data;
		
		l = l->next;
	}
	
	return NULL;
}

int hashtable_remove(hashtable_t* h, uint64_t key, destroyfunction destroy, void* data)
{
	assert(h);
	
	uint64_t hash = h->hash(key);
	unsigned elem = hash % h->size;
	_list_t* prev = h->elements[elem];

	_list_t* l = prev ? prev->next : NULL;
	if (prev)
	{
		if (prev->key == key)
		{
			if (destroy)
				if (!destroy(prev->key, prev->data, data))
					return 1;

			free(prev);
			h->elements[elem] = NULL;

			if (l)
				h->elements[elem] = l;

			return 1;
		}
	}
	while (l)
	{
		if (l->key == key)
		{
			_list_t* next = l->next;
			if (destroy)
				if (!destroy(l->key, l->data, data))
					return 1;

			free(l);
			prev->next = next;
			return 1;
		}
		
		prev = l;
		l = l->next;
	}
	
	return 0;
}
