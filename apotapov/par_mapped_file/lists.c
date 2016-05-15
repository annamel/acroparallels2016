#include "common_types.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "logger.h"

list_t *list_init() {

    list_t *buf = (list_t *)calloc(1, sizeof(list_t));

    if(buf == NULL) {
        write_log_to_file(Error, "list_init: problems with calloc in list_init!\n");
        return NULL;
    }

    buf -> size = 0;
    buf -> first = NULL;
    buf -> end = NULL;

    return buf;
}

int list_deinit(list_t *list) {

    if(list == NULL) {
        write_log_to_file(Error, "list_deinit: invalid input, list = NULL!\n");
        return EINVAL;
    }

    while(list -> size != 0) {
        list_erase_first(list);
    }

    free(list);

    return 0;
}

int list_add_last(list_t *list, data_t data) {

    if(list == NULL){
        write_log_to_file(Error, "list_add_last: invalid input, list = NULL!\n");
        return EINVAL;
    }

    elem_t *buf_elem = (elem_t *)calloc(1, sizeof(elem_t));
    if(buf_elem == NULL) {
        write_log_to_file (Error, "list_add_last: problems with calloc!\n");
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

    return 0;
}

int list_erase_first(list_t *list) {

    if(list == NULL) {
        write_log_to_file(Error, "list_erase_last: invalid input, list = NULL!\n");
        return EINVAL;
    }

    if(list -> size == 0) {
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
    
    return 0;
}
