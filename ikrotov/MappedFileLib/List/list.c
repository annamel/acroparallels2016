#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include <errno.h>
#include "../ChunkManager/ChunkManager.h"

list_t* list_init() {
	//log_write(DEBUG, "list_init: start of creation");

	list_t* new_list = (list_t*)calloc(1, sizeof(list_t));
	if(!new_list) {
		//log_write(ERROR, "list_init: cannot allocate memory");
		return NULL;
	}

	new_list->head = NULL;
	new_list->tail = NULL;
	new_list->size = 0;

	return new_list;
}

int list_add_last(list_t* list, value_t new_value) {
	//log_write(DEBUG, "list_add_last: start of working");

	if(!list) {
		//log_write(ERROR, "list_add_last: there is no list");
		return EINVAL;
	}

	list_item_t* new_item = (list_item_t*)calloc(1, sizeof(list_item_t));
	if(!new_item) {
		//log_write(ERROR, "list_add_last: cannot create new element");
		return ENOMEM;
	}

	new_item->value = new_value;

	if(list->size == 0) {
		new_item->next = NULL;
		new_item->prev = NULL;

		list->head = new_item;
		list->tail = new_item;
		list->size += 1;

		//log_write(DEBUG, "list_add_last: element added");
		return 0;
	} else {
		new_item->next = NULL;
		new_item->prev = list->tail;
		list->tail->next = new_item;
		list->tail = new_item;
		list->size += 1;
		//log_write(DEBUG, "list_add: element added");
		return 0;
	}
}

int list_find(list_t* list, value_t value, list_item_t** item) {
	//why do we need this func?
}

int list_delete_first(list_t* list) {
	//log_write(DEBUG, "list_delete_first: start of working");

	if(!list) {
		//log_write(ERROR, "list_delete_first: there is no list");
		return EINVAL;
	}

	if(list->size == 0) {
		//log_write(WARNING, "list_delete_first: empty list");
		return ENODATA;
	} else if (list->size == 1) {
		free(list->head);
		list->head = NULL;
		list->tail = NULL;
		list->size -= 1;
		//log_write(DEBUG, "list_delete_first: done");
	} else {
		list_item_t* ptr = list->head;
		list->head->next->prev = NULL;
		list->head = list->head->next;
		free(ptr);
		list->size -= 1;
		//log_write(DEBUG, "list_delete_first: done");
	}

	return 0;
}

int list_deinit(list_t* list) {
	//log_write(DEBUG, "list_deinit: start of working");

	if(!list) {
		//log_write(ERROR, "list_deinit: there is no list");
		return EINVAL;
	}

	while(list_delete_first(list) != ENODATA);

	free(list);

	return 0;
}