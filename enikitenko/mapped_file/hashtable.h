#ifndef HASHTABLE
#define HASHTABLE

#include <stddef.h>

typedef struct _list
{
	struct _list* next;
	void* key;
	void* data;

} _list_t;

typedef size_t (*hashfunction)(void* key);
typedef int (*keycomparator)(void* key1, void* key2);
typedef void (*destroyfunction)(void* key, void* value);

typedef struct hashtable
{
	_list_t** elements;
	size_t size;
	hashfunction hash;
	keycomparator comparator;

} hashtable_t;

void	hashtable_init		(hashtable_t* h, size_t size, hashfunction hash, keycomparator comparator);
void	hashtable_destroy	(hashtable_t* h, destroyfunction destroy);
int		hashtable_add		(hashtable_t* h, void* key, void* element);
void*	hashtable_get		(hashtable_t* h, void* key);
int		hashtable_remove	(hashtable_t* h, void* key, destroyfunction destroy);

#endif // HASHTABLE
