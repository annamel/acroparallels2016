//
//  list.h
//  mf_project
//
//  Created by IVAN MATVEEV on 15.04.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#ifndef list_h
#define list_h

#include <stdlib.h>
#include <malloc/malloc.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include "best_error_define_ever.h"

typedef struct Data {
    void *ptr;
    size_t number_first_page;
    size_t size_in_pages;
    unsigned int counter;
} Data;

typedef struct Node {
    struct Node *next;
    Data value;
} Node;

typedef struct HeaderList {
    Node *firstNode;
    size_t size;
} HeaderList;

ssize_t sizeList(HeaderList *list);
void free_data(Data value);
void print_data(Data value);
void displayList(HeaderList *list);
void clearList(HeaderList *list);
void removeList(HeaderList *list);
HeaderList *emptyList(void);
Node *createNode(Data val);
// удаляет узел, но не освобождает память по указателю Data.ptr
// в number_first_page и size_in_page записывает информацию о узле который удалил
int removeFirstNode(HeaderList *list, size_t *number_first_page, size_t *size_in_page);
// добавляет узел
int appendNode(HeaderList *list, Data val);
// возвращает указатель на  Data, которая внутри списка
Data *getNode(HeaderList *list, size_t number_first_page, size_t size_in_page);
// удаляет узел и освобождает память Data.ptr, если flag_free_data == true
int removeNode(HeaderList *list, size_t number_first_page, size_t size_in_page, int flag_free_data);


#endif /* list_h */
