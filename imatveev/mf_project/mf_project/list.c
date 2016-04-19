//
//  list.c
//  mf_project
//
//  Created by IVAN MATVEEV on 16.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//


#include "list.h"
extern size_t mempagesize;

ssize_t sizeList(HeaderList *list) {
    if (list == NULL)
        return -1;
    else
        return list->size;
}
void print_data(Data value) {
    printf("{ptr:%p, number: %zu, size: %zu}\n",
           value.ptr, value.number_first_page, value.size_in_pages);
}

void displayList(HeaderList *list) {
    if (list == NULL)
        return;
    Node *node = list->firstNode;
    printf("[");
    while (node != NULL) {
        print_data(node->value);
        node = node->next;
    }
    printf("]\n");
}
void free_data(Data value) {
    munmap(value.ptr, value.size_in_pages*mempagesize);
}
void clearList(HeaderList *list) {
    if (list == NULL)
        return;
    list->size = 0;
    Node *node = list->firstNode;
    Node *prev_node = node;
    while(node != NULL) {
        prev_node = node;
        node = node->next;
        free_data(prev_node->value);
        free(prev_node);
    }
    list->firstNode = NULL;
}
void removeList(HeaderList *list) {
    if (list == NULL)
        return;
    clearList(list);
    free(list);
}
HeaderList *emptyList(void) {
    HeaderList *list = malloc(sizeof(HeaderList));
    error_with_malloc(list == NULL, NULL);
    list->firstNode = NULL;
    list->size = 0;
    return list;
}
Node *createNode(Data val) {
    Node *new_node = malloc(sizeof(Node));
    error_with_malloc(new_node == NULL, NULL);
    new_node->value = val;
    new_node->next = NULL;
    return new_node;
}
int appendNode(HeaderList *list, Data val) {
    if (list == NULL)
        return -1;
    if (list->firstNode == NULL) {
        Node *new_node = createNode(val);
        error_with_malloc(new_node == NULL, -1);
        list->firstNode = new_node;
    } else {
        Node *node = list->firstNode;
        while (node->next != NULL) {
            if (node->value.number_first_page == val.number_first_page &&
                node->value.size_in_pages == val.size_in_pages){
                node->value = val;
                return 0;
            }
            node = node->next;
        }
        Node *new_node = createNode(val);
        error_with_malloc(new_node == NULL, -1);
        node->next = new_node;
    }
    list->size++;
    return 0;
}
Data *getNode(HeaderList *list, size_t number_first_page, size_t size_in_page) {
    if (list == NULL)
        return NULL;
    Node *node = list->firstNode;
    while (node != NULL && number_first_page != node->value.number_first_page &&
           size_in_page != node->value.size_in_pages) {
        node = node->next;
    }
    return (node == NULL) ? NULL : &(node->value);
}
int removeNode(HeaderList *list, size_t number_first_page, size_t size_in_page, int flag_free_data) {
    if (list == NULL)
        return -1;
    Node *node = list->firstNode;
    if (node != NULL && node->value.number_first_page == number_first_page
                     && node->value.size_in_pages == size_in_page){
        list->firstNode = node->next;
        free_data(node->value);
        free(node);
    } else {
        Node *prev_node = node;
        while (node != NULL && !(node->value.number_first_page == number_first_page
                              && node->value.size_in_pages == size_in_page)) {
            prev_node = node;
            node = node->next;
        }
        if (node == NULL){
            errno = EINVAL;
            return -2;
        }
        prev_node->next = node->next;
        if (flag_free_data)
            free_data(node->value);
        free(node);
    }
    list->size--;
    return 0;
}
int removeFirstNode(HeaderList *list, size_t *number_first_page, size_t *size_in_page){
    if (list == NULL)
        return -1;
    Node *node = list->firstNode;
    if (node == NULL)
        return -1;
    list->firstNode = node->next;
    *number_first_page = node->value.number_first_page;
    *size_in_page = node->value.size_in_pages;
    free(node);
    return 0;
}







