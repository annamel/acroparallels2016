//
//  i_list.h
//  second_try_mf_lib
//
//  Created by IVAN MATVEEV on 24.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#ifndef i_list_h
#define i_list_h


#include "list.h"

typedef struct iNode{
    Node node;
    struct iNode *next;
    struct iNode *prev;
} iNode;

typedef struct iList {
    iNode *first_inode;
    iNode *last_inode;
} iList;

iList *_init_empty_ilist(void);
// RETURN VALUE: 0 - success, -1 - failed
int init_empty_ilist(iList *list);
// append as first Node
int ilist_append(iList *list, Node *new_node);
int ilist_remove(iList *list, Node *node);
// delete from list and return node
iNode *ilist_get_first(iList *list);
iNode *ilist_get_last(iList *list);
int ilist_is_empty(iList *list);
ssize_t ilist_size(iList *list);

#endif /* i_list_h */
