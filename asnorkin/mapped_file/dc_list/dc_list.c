#include "dc_list.h"


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "../logger/logger.h"



dclist_t *dcl_init()
{
    //log_write(DEBUG, "dcl_init: started");

    dclist_t *list = (dclist_t *)calloc(1, sizeof(dclist_t));
    if(!list)
    {
        //log_write(ERROR, "dcl_init: can't allocate memory");
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    //log_write(DEBUG, "dcl_init: finished");

    return list;
}



int dcl_add_last(dclist_t *list, lvalue_t value)
{
    if(!list)
        return EINVAL;
    //log_write(DEBUG, "dcl_add_last(value=%d): started", value);


    dcl_item_t *item = (dcl_item_t *)calloc(1, sizeof(dcl_item_t));
    if(!item)
    {
        //log_write(ERROR, "dcl_add_last(value=%d): can't allocate memory for item", value);
        return ENOMEM;
    }

    item->value = value;

    if(!list->size)
    {
        item->next = NULL;
        item->prev = NULL;

        list->head = item;
        list->tail = item;
    }
    else
    {
        item->next = NULL;
        item->prev = list->tail;
        item->prev->next = item;
        list->tail = item;
    }
    list->size ++;

    //log_write(DEBUG, "dcl_add_last(value=%d): finished", value);
    return 0;
}



int dcl_del_first(dclist_t *list)
{
    if(!list)
        return EINVAL;

    //log_write(DEBUG, "dcl_del_first: started");

    if(!list->size)
    {
        //log_write(INFO, "dcl_del_first: trying del from empty list");
        return ENODATA;
    }

    if(list->size == 1)
    {
        free(list->head);
        list->head = NULL;
        list->tail = NULL;
    }
    else
    {
        dcl_item_t *del_ptr = list->head;
        list->head = list->head->next;
        free(del_ptr);
        del_ptr = NULL;
        list->head->prev = NULL;
    }
    list->size --;

    //log_write(DEBUG, "dcl_del_first: finished");
    return 0;
}



int dcl_del_by_value(dclist_t *list, lvalue_t value)
{
    if(!list)
        return EINVAL;
    //log_write(INFO, "dcl_del_by_value: started");    


    dcl_item_t *curr_item = list->head;
    if(curr_item && curr_item->value == value)
        return dcl_del_first(list);


    while(curr_item && curr_item->next)
    {
        if(curr_item->value == value)
            break;

        curr_item = curr_item->next;
    }


    if(curr_item && curr_item->value == value)
    {
        if(curr_item->next)
        {            
            curr_item->next->prev = curr_item->prev;
            curr_item->prev->next = curr_item->next;
        }
        else
        {
            curr_item->prev->next = NULL;
            list->tail = curr_item->prev;
        }

        free(curr_item);
        curr_item = NULL;

        list->size--;
        //log_write(INFO, "dcl_del_by_value: finished");
        return 0;
    }


    //log_write(INFO, "dcl_del_by_value: finished");
    return ENOKEY;
}



int dcl_deinit(dclist_t *list)
{
    if(!list)
        return EINVAL;

    //log_write(DEBUG, "dcl_deinit: started");

    while(dcl_del_first(list) != ENODATA);

    free(list);

    //log_write(DEBUG, "dcl_deinit: finished");
    return 0;
}



void dcl_print(dclist_t *list, int to)
{
    if(!list)
        printf("Trying to print NULL list\n");

    printf("\n******************************");
    printf("\n**********   List   **********");
    printf("\n******************************\n");
    dcl_item_t *curr_item = list->head;
    for(int i = 0; i < to; i++)
    {        
        printf("[%d], value: %u\n", i, curr_item->value);
        curr_item = curr_item->next;
    }
    printf("*** head: %u\n", list->head->value);
    printf("*** tail: %u\n", list->tail->value);
    printf("*** size: %u\n", list->size);
    printf("******************************\n");

    return;
}
