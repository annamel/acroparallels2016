#ifndef DC_LIST_H
#define DC_LIST_H



typedef struct dc_list dclist_t;
typedef struct dcl_item dcl_item_t;
typedef void * lvalue_t;



struct dc_list
{
    unsigned int size;
    dcl_item_t *head;
    dcl_item_t *tail;
};



struct dcl_item
{
    dcl_item_t *next;
    dcl_item_t *prev;
    lvalue_t value;
};




dclist_t *dcl_init();
int dcl_add_last(dclist_t *list, lvalue_t value);
int dcl_del_first(dclist_t *list);
int dcl_del_by_value(dclist_t *list, lvalue_t value);
int dcl_deinit(dclist_t *list);
void dcl_print(dclist_t *list);



#endif // DC_LIST_H
