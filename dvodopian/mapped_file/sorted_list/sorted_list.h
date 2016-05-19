//
// Created by voddan on 22/04/16.
//

#ifndef FILE_MAPPER_PROJECT_ORDERED_LIST_H
#define FILE_MAPPER_PROJECT_ORDERED_LIST_H


#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct sorted_list sorted_list;

/* natural order */
typedef int comparator_t(void * item);



sorted_list * list_alloc(comparator_t* func);

void list_free(sorted_list * lp);


void list_add(sorted_list * lp, void * item);

/* does nothing on index-out-of-range */
void list_del(sorted_list * lp, int index);


/* returns NULL on index-out-of-range */
void * list_get(sorted_list * lp, int index);

int list_size(sorted_list * lp);


/* returns -1 if nothing found */
int list_search(sorted_list* lp, void * item);

void list_remove(sorted_list * lp, void * item);



#ifdef __cplusplus
}
#endif

#endif //FILE_MAPPER_PROJECT_ORDERED_LIST_H
