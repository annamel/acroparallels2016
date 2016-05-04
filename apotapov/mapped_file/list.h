#ifndef list_h
#define list_h

#include "common_types.h"

list_t *list_init();
int list_deinit(list_t *list);
int list_add_last(list_t *list, data_t data);
int list_erase_first(list_t *list);

#endif

