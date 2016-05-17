//
//  list.c
//  second_try_mf_lib
//
//  Created by IVAN MATVEEV on 23.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int list_is_empty(List *list){
    if (list == NULL)
        return -1;
    else
        return (list->first_node == NULL) ? 1 : 0;
}
int init_empty_list(List *list){
    if (list == NULL)
        return -1;
    list->first_node = NULL;
    list->last_node = NULL;
    return 0;
}
int list_append(List *list, Node *new_node){
    if (list == NULL || new_node == NULL)
        return -1;
    new_node->prev = NULL;
    new_node->next = list->first_node;
    if (list->first_node == NULL){
        list->last_node = new_node;
    } else {
        list->first_node->prev = new_node;
    }
    list->first_node = new_node;
    return 0;
}
int list_remove(List *list, Node *node){
    if (list == NULL || node == NULL)
        return -1;
    if (list->first_node == node){
        list->first_node = node->next;
        if (node->next != NULL)
            node->next->prev = NULL;
    } else {
        if (list->last_node == node){
            list->last_node = node->prev;
            if (node->prev != NULL)
                node->prev->next = NULL;
        } else {
            if (node->next != NULL)
                node->next->prev = node->prev;
            if (node->prev != NULL)
                node->prev->next = node->next;
        }
    }
    return 0;
}
Node *list_remove_first(List *list){
    if (list == NULL || list->first_node == NULL)
        return NULL;
    Node *node = list->first_node;
    list->first_node = node->next;
    if (node->next != NULL)
        node->next->prev = NULL;
    else
        list->last_node = NULL;
    return node;
}
Node *list_remove_last(List *list){
    if (list == NULL || list->last_node == NULL)
        return NULL;
    Node *node = list->last_node;
    list->last_node = node->prev;
    if (node->prev != NULL)
        node->prev->next = NULL;
    else
        list->first_node = NULL;
    return node;
}
ssize_t list_size(List *list){
    ssize_t size = 0;
    if (list == NULL)
        return -1;
    Node * node = list->first_node;
    while (node != NULL){
        size++;
        node = node->next;
    }
    return size;
}
Node *list_find(List *list, int (*check)(Data)){
    if (list == NULL)
        return NULL;
    Node *node = list->first_node;
    while (node != NULL && !check(node->value))
        node = node->next;
    return node;
}



