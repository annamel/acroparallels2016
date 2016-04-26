#ifndef list_h
#define list_h

typedef struct element elem_t;
typedef struct list list_t;
typedef chunk_t* data_t

struct element {
    elem_t *next;
    elem_t *prev;
    data_t data;
};

struct list {
	elem_t *first;
    elem_t *end;
    unsigned int size;  
};

list_t *list_init();
int list_deinit(list_t *list);
int list_add_last(list_t *list, data_t data);
int list_erase_first(list_t *list);
//void print_list(list_t *list);
//int list_deinit_first(list_t *list);

#endif