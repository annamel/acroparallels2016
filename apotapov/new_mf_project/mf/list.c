#include "list.h"
#include "logger.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


list_t *list_init() {
    write_log(Debug, "list_init: started!\n");
    list_t *buf = (list_t *)calloc(1, sizeof(list_t));
    if(buf == NULL) {
        write_log(Error, "list_init: problems with calloc in list_init!\n");
        return NULL;
    }
    buf -> size = 0;
    buf -> first = NULL;
    buf -> end = NULL;
    write_log (Info, "list_init: finished!\n");
    return buf;
}

int list_deinit(list_t *list) {
    write_log(Info, "list_deinit: started!\n");
    if(list == NULL) {
        write_log(Error, "list_deinit: invalid input, list = NULL!\n");
        return EINVAL;
    }
    while(list -> size != 0) {
        list_erase_first(list);
    }
    free(list);
    write_log(Info, "list_deinit: finished!\n");
    return 0;
}

int list_add_last(list_t *list, data_t data) {
    write_log (Info, "list_add_last: started!\n");
    if(list == NULL){
        write_log(Error, "list_add_last: invalid input, list = NULL!\n");
        return EINVAL;
    }
    elem_t *buf_elem = (elem_t *)calloc(1, sizeof(elem_t));
    if(buf_elem == NULL) {
        write_log (Error, "list_add_last: problems with calloc!\n");
        return ENOMEM;
    }
    buf_elem -> next = NULL;
    buf_elem -> prev = NULL;
    buf_elem -> data = data;

    if(list -> size == 0) {
        buf_elem -> next = NULL;
        buf_elem -> prev = NULL;
        list -> first = buf_elem;
        list -> end = buf_elem;
    }
    else {
        buf_elem -> next = NULL;
        buf_elem -> prev = list -> end;
        buf_elem -> prev -> next = buf_elem;
        list -> end = buf_elem;
    }
    list -> size += 1;
    write_log (Info, "list_add_last: finished!\n");
    return 0;
}

int list_erase_first(list_t *list) {
    write_log (Info, "list_erase_first: started!\n");
    if(list == NULL) {
        write_log(Error, "list_erase_last: invalid input, list = NULL!\n");
        return EINVAL;
    }
    if(list -> size == 0) {
        write_log (Info, "list_erase_first: list is empty!\n");
        return ENODATA;
    }

    if(list -> size == 1) {
        free(list -> first);
        list -> end = NULL;
        list -> first = NULL;
    }
    else {
        elem_t *ptr = list -> first;
        list -> first = list -> first -> next;
        free(ptr);
        list -> first -> prev = NULL;
    }
    list -> size -= 1;
    write_log (Info, "list_erase_first: finished!\n");
    return 0;
}


/*int list_deinit_first(list_t *list)
{
    if(list == NULL) {
        return EINVAL;
    }

    write_log (Debug, "list_erase_first: started!");

    if(list -> size == 0) {
        write_log (Info, "list_erase_first: list is empty!");
        return ENODATA;
    }

    if(list -> size == 1) {
        chunk_deinit(list -> first -> data);
        free(list -> first);
        list -> end = NULL;
        list -> first = NULL;
    }
    else {
        elem_t *ptr = list -> first;
        list -> first = list -> first -> next;
        chunk_deinit(ptr -> data);
        free(ptr);
        list -> first -> prev = NULL;
    }
    list -> size -= 1;
    write_log (Debug, "list_erase_first: finished!");
    return 0;
}

void print_list(list_t *list)
{
    if(list == NULL)
        printf("List is not initialized!\n");

    elem_t *elem = list -> first;
    int i = 0;
    for(i = 0; i < list -> size; i++) {
        printf("----------------\n");
        printf("number = %d\n", i);
        printf("data: %d\n", elem -> data);
        printf("----------------\n");
        elem = elem -> next;
    }
}*/