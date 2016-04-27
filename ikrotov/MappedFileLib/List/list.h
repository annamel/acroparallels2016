#ifndef LIST_H
#define LIST_H
#include "../ChunkManager/ChunkManager.h"

typedef void* value_t;

typedef struct list_item list_item_t;
typedef struct list list_t;

struct list_item
{
	list_item_t* next;
	list_item_t* prev;
	value_t value;
};

struct list
{
	list_item_t* head;
	list_item_t* tail;
	unsigned size;
};


list_t* list_init();
int list_add_last(list_t* list, value_t new_value);
int list_delete_first(list_t* list);
int list_deinit(list_t* list);

#endif


