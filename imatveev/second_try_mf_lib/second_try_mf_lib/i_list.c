//
//  i_list.c
//  second_try_mf_lib
//
//  Created by IVAN MATVEEV on 24.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#include "i_list.h"
#include <stdlib.h>
#include <errno.h>

#define inode_from_node(_node) ((iNode*)((void *)(_node) - (void*)&(((iNode *)NULL)->node)))
int ilist_is_empty(iList *list){
    if (list == NULL)
        return -1;
    else
        return (list->first_inode == NULL) ? 1 : 0;
}
iList *_init_empty_ilist(void){
    iList *list = malloc(sizeof(iList));
    if (list == NULL){
        errno = ENOMEM;
        return NULL;
    }
    list->first_inode = NULL;
    list->last_inode = NULL;
    return list;
}
int init_empty_ilist(iList *list){
    if (list == NULL)
        return -1;
    list->first_inode = NULL;
    list->last_inode = NULL;
    return 0;
}
int ilist_append(iList *list, Node *new_node){
    if (list == NULL || new_node == NULL)
        return -1;
    iNode *new_inode = inode_from_node(new_node);
    if (list->first_inode == NULL){
        list->first_inode = new_inode;
        list->last_inode = new_inode;
        new_inode->next = NULL;
        new_inode->prev = NULL;
    } else {
        iNode *inode = list->first_inode;
        inode->prev = new_inode;
        new_inode->next = inode;
        new_inode->prev = NULL;
        list->first_inode = new_inode;
    }
    return 0;
}
int ilist_remove(iList *list, Node *node){
    if (list == NULL || node == NULL)
        return -1;
    iNode *inode = inode_from_node(node);
    if (list->first_inode == inode){
        list->first_inode = inode->next;
        if (inode->next != NULL)
            inode->next->prev = NULL;
    } else {
        if (list->last_inode == inode){
            list->last_inode = inode->prev;
            if (inode->prev != NULL)
                inode->prev->next = NULL;
        } else {
            if (inode->next != NULL)
                inode->next->prev = inode->prev;
            if (inode->prev != NULL)
                inode->prev->next = inode->next;
        }
    }
    return 0;
}
iNode *ilist_get_first(iList *list){
    if (list == NULL || list->first_inode == NULL)
        return NULL;
    iNode *inode = list->first_inode;
    list->first_inode = inode->next;
    if (inode->next != NULL)
        inode->next->prev = NULL;
    else
        list->last_inode = NULL;
    return inode;
}
iNode *ilist_get_last(iList *list){
    if (list == NULL || list->last_inode == NULL)
        return NULL;
    iNode *inode = list->last_inode;
    list->last_inode = inode->prev;
    if (inode->prev != NULL)
        inode->prev->next = NULL;
    else
        list->first_inode = NULL;
    return inode;
}
ssize_t ilist_size(iList *list){
    ssize_t size = 0;
    if (list == NULL)
        return -1;
    iNode * inode = list->first_inode;
    while (inode != NULL){
        size++;
        inode = inode->next;
    }
    return size;
}



