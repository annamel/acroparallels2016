#include <stdio.h>

#include "hash_table.h"
#include "hash_funcs.h"

int main(void)
{
    int error = 0;

    htable_t *htable = ht_init(10, hash_fnv1a);
    if(!htable)
        printf("hash table init has failed\n");



    printf("\n***add item tests***\n");
    error = ht_add_item(htable, 10, 100);
    if(error)
        printf("can't add item: %d\n", error);

    error = ht_add_item(htable, 0, 0);
    if(error)
        printf("can't add item: %d\n", error);

    error = ht_add_item(htable, -10, 1003254);
    if(error)
        printf("can't add item: %d\n", error);

    error = ht_add_item(htable, 10, -999999);
    if(error)
        printf("can't add item: %d\n", error);



    printf("\n***find by key tests***\n");
    item_t *tmp_item = NULL;
    error = ht_find_by_key(htable, 10, &tmp_item);
    if(error)
        printf("can't find by key: %d\n", error);
    else
        printf("finded value: %d\n", tmp_item->value);

    error = ht_find_by_key(htable, 10, &tmp_item);
    if(error)
        printf("can't find by key: %d\n", error);
    else
        printf("finded value: %d\n", tmp_item->value);

    error = ht_find_by_key(htable, 10, &tmp_item);
    if(error)
        printf("can't find by key: %d\n", error);
    else
        printf("finded value: %d\n", tmp_item->value);

    error = ht_find_by_key(htable, -10, &tmp_item);
    if(error)
        printf("can't find by key: %d\n", error);
    else
        printf("finded value: %d\n", tmp_item->value);

    error = ht_find_by_key(htable, 5, &tmp_item);
    if(error)
        printf("can't find by key: %d\n", error);
    else
        printf("finded value: %d\n", tmp_item->value);



    printf("\n***find by key and value tests***\n");
    error = ht_find_by_kav(htable, 10, 100, &tmp_item);
    if(error)
        printf("can't find by key and value: %d\n", error);
    else
        printf("finded value: %d\n", tmp_item->value);

    error = ht_find_by_kav(htable, 10, 0, &tmp_item);
    if(error)
        printf("can't find by key and value: %d\n", error);
    else
        printf("finded value: %d\n", tmp_item->value);

    error = ht_find_by_kav(htable, -5, 3, &tmp_item);
    if(error)
        printf("can't find by key and value: %d\n", error);
    else
        printf("finded value: %d\n", tmp_item->value);



    printf("\ndelete by key tests\n");
    error = ht_del_item_by_key(htable, 10);
    if(error)
        printf("can't delete by key: %d\n", error);

    error = ht_del_item_by_key(htable, 10);
    if(error)
        printf("can't delete by key: %d\n", error);

    error = ht_del_item_by_key(htable, 10);
    if(error)
        printf("can't delete by key: %d\n", error);



    error = ht_deinit(htable);
    if(error)
        printf("\nhash table deinit has failed: %d\n", error);

    printf("\nHash table testing has finished\n");

    return 0;
}

