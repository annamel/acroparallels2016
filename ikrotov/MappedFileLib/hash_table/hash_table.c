#include "hash_table.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>



htable_t *ht_init(const int size, hfunc_t *hash_func)
{
    if(!hash_func)
        return NULL;

    htable_t *hash_table = (htable_t *)calloc(1, sizeof(htable_t));
    if(!hash_table)
        return NULL;

    hash_table->table = (item_t **)calloc(size, sizeof(item_t *));
    if(!hash_table->table)
    {
        free(hash_table);
        return NULL;
    }

    hash_table->size = size;
    hash_table->hash_func = hash_func;

    return hash_table;
}



int ht_deinit(htable_t *ht)
{
    if(!ht)
        return EINVAL;

    int i = 0;
    for(i = 0; i < ht->size; i++)
    {
        item_t *del_ptr1 = ht->table[i];
        item_t *del_ptr2 = ht->table[i];

        if(del_ptr1 == NULL)
        {
            free(del_ptr1);
            continue;
        }
        else
        {
            while(del_ptr2 != NULL)
            {
                del_ptr1 = del_ptr2;
                del_ptr2 = del_ptr2->next;
                free(del_ptr1);
            }
            continue;
        }
    }
    return 0;
}



int ht_add_item(htable_t *ht, hkey_t key, hvalue_t value)
{
    if(!ht)
        return EINVAL;

    if(ht_find_by_kav(ht, key, value, NULL) != ENOKEY)
        return EKEYREJECTED;

    int index = ht->hash_func(key, HASH_CONST_2) % ht->size;

    item_t *new_item_ptr = (item_t *)calloc(1, sizeof(item_t));
    if(!new_item_ptr)
        return ENOMEM;

    new_item_ptr->key = key;
    new_item_ptr->value = value;
    new_item_ptr->next = NULL;

    if(!ht->table[index])
    {
        ht->table[index] = new_item_ptr;
        ht->table[index]->prev = NULL;
        return 0;
    }
    else
    {
        item_t *ptr = ht->table[index];
        while(ptr->next)
            ptr = ptr->next;

        ptr->next = new_item_ptr;
        ptr->next->prev = ptr;

        return 0;
    }

    return -1;
}



int ht_find_by_key(htable_t *ht, const hkey_t key, item_t **item)
{
    if(!ht)
        return EINVAL;

    int index = ht->hash_func(key, HASH_CONST_2) % ht->size;
    item_t *item_ptr = ht->table[index];
    if(!item_ptr)
        return ENOKEY;

    while(item_ptr)
    {
        if(item_ptr->key == key)
        {
            *item = item_ptr;
            return 0;
        }
        else
        {
            item_ptr = item_ptr->next;
            continue;
        }
    }

    return ENOKEY;
}



int ht_find_by_kav(htable_t *ht, hkey_t key, hvalue_t *value, item_t **item)
{
    if(!ht)
        return EINVAL;

    int index = ht->hash_func(key, HASH_CONST_2) % ht->size;
    item_t *item_ptr = ht->table[index];
    if(!item_ptr)
        return ENOKEY;

    while(item_ptr)
    {
        if(item_ptr->key == key, item_ptr->value == value)
        {
            *item = item_ptr;
            return 0;
        }
        else
        {
            item_ptr = item_ptr->next;
            continue;
        }
    }

    return ENOKEY;
}



int ht_del_item_by_key(htable_t *ht, hkey_t key)
{
    if(!ht)
        return EINVAL;

    int index = ht->hash_func(key, HASH_CONST_2) % ht->size;

    //Case 0 - bucket is empty
    if(!ht->table[index])
        return ENOKEY;

    //Case 1 - only 1 item contained in bucket
    //         and that item has matching name
    else if(ht->table[index]->key == key &&
            ht->table[index]->next == NULL)
    {
        free(ht->table[index]);
        ht->table[index] = NULL;
        return 0;
    }

    //Case 2 - match is located in the first item in the
    //         bucket but there are more items in the bucket
    else if(ht->table[index]->key == key)
    {
        item_t *delPtr = ht->table[index];
        ht->table[index] = ht->table[index]->next;
        ht->table[index]->prev = NULL;
        free(delPtr);
        return 0;
    }

    //Case 3 - bucket contains items but first item is not a match
    else
    {
        item_t *ptr1 = ht->table[index]->next;
        item_t *ptr2 = ht->table[index];

        while(ptr1 != NULL && ptr1->key != key)
        {
            ptr2 = ptr1;
            ptr1 = ptr1->next;
        }
        //Case 3.1 : no match
        if(ptr1 == NULL)
            return ENOKEY;
        //Case 3.2 : match is found
        else
        {
            ptr2->next = ptr1->next;
            free(ptr1);
            return 0;
        }
    }
}



int ht_del_item_by_kav(htable_t *ht, hkey_t key, hvalue_t value)
{
    if(!ht)
        return EINVAL;

    int index = ht->hash_func(key, HASH_CONST_2) % ht->size;

    //Case 0 - bucket is empty
    if(!ht->table[index])
        return ENOKEY;

    //Case 1 - only 1 item contained in bucket
    //         and that item has matching name
    else if(ht->table[index]->key == key &&
            ht->table[index]->value == value &&
            ht->table[index]->next == NULL)
    {
        free(ht->table[index]);
        ht->table[index] = NULL;
        return 0;
    }

    //Case 2 - match is located in the first item in the
    //         bucket but there are more items in the bucket
    else if(ht->table[index]->key == key &&
            ht->table[index]->value == value)
    {
        item_t *delPtr = ht->table[index];
        ht->table[index] = ht->table[index]->next;
        free(delPtr);
        return 0;
    }

    //Case 3 - bucket contains items but first item is not a match
    else
    {
        item_t *ptr1 = ht->table[index]->next;
        item_t *ptr2 = ht->table[index];

        while(ptr1 != NULL && ptr1->key != key &&
              ptr1->value != value)
        {
            ptr2 = ptr1;
            ptr1 = ptr1->next;
        }
        //Case 3.1 : no match
        if(ptr1 == NULL)
            return ENOKEY;
        //Case 3.2 : match is found
        else
        {
            ptr2->next = ptr1->next;
            free(ptr1);
            return 0;
        }
    }
}



void print_table(htable_t *ht)
{
    if(!ht)
    {
        printf("print_table::ERROR::hash_table pointer is NULL\n");
        return;
    }

    int i = 0;
    for(i = 0; i < ht->size; i++)
    {
        if(ht->table[i] == NULL)
        {
            printf("----------------\n");
            printf("index = %d\n", i);
            printf("key : empty\n");
            printf("value : empty\n");
            printf("----------------\n");
        }
        else
        {
            printf("----------------\n");
            printf("index = %d\n", i);
            printf("key : %llu\n", ht->table[i]->key);
            printf("value : %u\n", ht->table[i]->value);
            printf("# of items = %d\n",
                   number_of_items_in_index(ht, i));
            printf("----------------\n");
        }
    }
}



int print_list_in_index(htable_t *ht, int index)
{
    if(!ht || index >= ht->size)
        return EINVAL;

    item_t *curr_item_ptr = (item_t *)calloc(1, sizeof(item_t));
    curr_item_ptr = ht->table[index];

    printf("****************\n");
    printf("Row %d ", index);

    if(curr_item_ptr == NULL)
    {
        printf("is empty\n");
        printf("****************\n");
        return 0;
    }
    else
    {
        printf("\n****************\n");
        int i = 0;

        while(curr_item_ptr != NULL)
        {
            printf("****************\n");
            printf("sequence number = %d\n", i++);
            printf("key: %llu\n", curr_item_ptr->key);
            printf("value: %u\n", curr_item_ptr->value);
            printf("****************\n");

            curr_item_ptr = curr_item_ptr->next;
        }

        return 0;
    }

    return -1;
}



int number_of_items_in_index(htable_t *ht, int index)
{
    if(!ht || index >= ht->size)
        return EINVAL;

    item_t *ptr = ht->table[index];
    uint32_t count = 0;

    while(ptr)
    {
        ptr = ptr->next;
        count++;
    }

    return count;
}
