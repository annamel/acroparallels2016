//
//  list.h
//  second_try_mf_lib
//
//  Created by IVAN MATVEEV on 23.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#ifndef list_h
#define list_h

#include <stdio.h>

typedef struct Data {
    void *ptr;
    size_t number_first_page;
    size_t size_in_pages;
    size_t counter;
} Data;

typedef struct Node {
    struct Node *next;
    struct Node *prev;
    Data value;
} Node;

typedef struct List {
    Node *first_node;
    Node *last_node;
} List;

// RETURN VALUE: 0 - success, -1 - failed
int init_empty_list(List *list);
// append as first Node
int list_append(List *list, Node *new_node);
int list_remove(List *list, Node *node);
// delete from list and return node
Node *list_remove_first(List *list);
Node *list_remove_last(List *list);
int list_is_empty(List *list);
ssize_t list_size(List *list);
// check - возвращает 1 если чанк подходит, 0 если нет
Node *list_find(List *list, int (*check)(Data));

#endif /* list_h */


