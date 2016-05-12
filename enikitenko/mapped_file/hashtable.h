#ifndef HASHTABLE
#define HASHTABLE

#include <stddef.h>
#include <stdint.h>

typedef struct _list
{
	struct _list* next;
	uint64_t key;
	void* data;

} _list_t;

typedef uint64_t (*hashfunction)(uint64_t);
typedef int (*destroyfunction)(uint64_t, void* value, void* data);

typedef struct hashtable
{
	_list_t** elements;
	size_t size;
	hashfunction hash;

} hashtable_t;

void	hashtable_init		(hashtable_t* h, size_t size, hashfunction hash);
void	hashtable_destroy	(hashtable_t* h, destroyfunction destroy, void* data);
int		hashtable_add		(hashtable_t* h, uint64_t key, void* element);
void*	hashtable_get		(hashtable_t* h, uint64_t key);
int		hashtable_remove	(hashtable_t* h, uint64_t key, destroyfunction destroy, void* data);

#endif // HASHTABLE
