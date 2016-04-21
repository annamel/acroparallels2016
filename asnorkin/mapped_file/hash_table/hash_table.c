//*********************************************************
//********                                       **********
//********   Created by asnorkin on 22.03.2016.  **********
//********                                       **********
//*********************************************************

#include "hash_table.h"



hash_table_t *hash_table_init(const int size,
                              hash_func_t *hash_func)
{
    hash_table_t *hash_table = (hash_table_t *)calloc(1, sizeof(hash_table_t));
    if(hash_table == NULL)
    {
        return NULL;
    }

    hash_table->table = (item_t **)calloc(size, sizeof(struct item_t *));
    if(hash_table->table == NULL)
    {
        free(hash_table);
        return NULL;
    }

    hash_table->size = size;
    hash_table->hash_func = hash_func;

    return hash_table;
}



int hash_table_deinit(hash_table_t *hash_table)
{
    if(hash_table == NULL)
        return EINVAL;

    int i = 0;
    for(i = 0; i < hash_table->size; i++)
    {
        item_t *del_ptr1 = hash_table->table[i];
        item_t *del_ptr2 = hash_table->table[i];

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



void print_table(hash_table_t *hash_table)
{
    if(hash_table == NULL)
    {
        printf("print_table::ERROR::hash_table pointer is NULL\n");
        return;
    }

    int i = 0;
    for(i = 0; i < hash_table->size; i++)
    {
        if(hash_table->table[i] == NULL)
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
            printf("key : %llu\n", hash_table->table[i]->key);
            printf("value : %u\n", hash_table->table[i]->value);
            printf("# of items = %d\n",
                   number_of_items_in_index(hash_table, i));
            printf("----------------\n");
        }
    }
}



void print_list_in_index(hash_table_t *hash_table, int index)
{
    if(hash_table == NULL)
    {
        printf("print_list_in_index::ERROR::hash_table pointer is NULL\n");
        return;
    }
    else if(index >= hash_table->size)
    {
        printf("print_list_in_index::ERROR::index is out of range");
        return;
    }

    item_t *curr_item_ptr = (item_t *)calloc(1, sizeof(item_t));
    curr_item_ptr = hash_table->table[index];

    printf("****************\n");
    printf("Row %d ", index);

    if(curr_item_ptr == NULL)
    {
        printf("is empty\n");
        printf("****************\n");
        return;
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

        return;
    }

    return;
}



int add_item(hash_table_t *hash_table,
                 hash_key_t new_key, value_t new_value)
{
    value_t temporary_value = 0;
    if(hash_table == NULL ||
            find_value(hash_table, new_key, &temporary_value) != ENOKEY)
        return EINVAL;

    int index = hash_table->hash_func(new_key, HASH_CONST_2) % hash_table->size;

    item_t *new_item_ptr = (item_t *)calloc(1, sizeof(item_t));
    if(new_item_ptr == NULL)
        return ENOMEM;

    new_item_ptr->key = new_key;
    new_item_ptr->value = new_value;
    new_item_ptr->next = NULL;

    if(hash_table->table[index] == NULL)
    {
        hash_table->table[index] = new_item_ptr;
        return 0;
    }
    else
    {
        item_t *ptr = hash_table->table[index];
        while(ptr->next)
            ptr = ptr->next;

        ptr->next = new_item_ptr;
        return 0;
    }

    return EAGAIN;
}



int number_of_items_in_index(hash_table_t *hash_table,
                             int index)
{
    if(hash_table == NULL)
        return -1;
    if(index >= hash_table->size)
        return -2;
    else
    {
        item_t *ptr = hash_table->table[index];
        uint32_t count = 0;

        while(ptr)
        {
            ptr = ptr->next;
            count++;
        }
        return count;
    }
}



int find_value(hash_table_t *hash_table,
                   const hash_key_t key, value_t *value)
{    
    if(!hash_table || !value)
        return EINVAL;

    int index = hash_table->hash_func(key, HASH_CONST_2) % hash_table->size;
    item_t *item_ptr = hash_table->table[index];

    if(item_ptr == NULL)
        return ENOKEY;
    else
    {
        while(item_ptr != NULL)
        {
            if(item_ptr->key == key)
            {
                *value = item_ptr->value;
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
}



int delete_item(hash_table_t *hash_table, hash_key_t key)
{
    if(hash_table == NULL)
        return EINVAL;

    int index = hash_table->hash_func(key, HASH_CONST_2) % hash_table->size;

    //Case 0 - bucket is empty
    if(hash_table->table[index] == NULL)
        return ENOKEY;

    //Case 1 - only 1 item contained in bucket
    //         and that item has matching name
    else if(hash_table->table[index]->key == key &&
            hash_table->table[index]->next == NULL)
    {
        free(hash_table->table[index]);
        return 0;
    }

    //Case 2 - match is located in the first item in the
    //         bucket but there are more items in the bucket
    else if(hash_table->table[index]->key == key)
    {
        item_t *delPtr = hash_table->table[index];
        hash_table->table[index] = hash_table->table[index]->next;
        free(delPtr);
        return 0;
    }

    //Case 3 - bucket contains items but first item is not a match
    else
    {
        item_t *ptr1 = hash_table->table[index]->next;
        item_t *ptr2 = hash_table->table[index];

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



hash_key_t hash_fnv1a(const hash_key_t key, uint32_t hash)
{
    const unsigned char* ptr = (const unsigned char*) &key;
    hash = fnv1a(*ptr++, hash);
    hash = fnv1a(*ptr++, hash);
    hash = fnv1a(*ptr++, hash);
    return fnv1a(*ptr  , hash);
}



uint32_t fnv1a(unsigned char oneByte, uint32_t hash)
{
    return (oneByte ^ hash) * HASH_CONST_1;
}
