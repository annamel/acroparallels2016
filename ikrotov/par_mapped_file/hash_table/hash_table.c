#include "hash_table.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

hkey_t hash_fnv1a(const hvalue_t key, uint32_t hash)
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

htable_t *ht_init(int size, hfunc_t *hash_func)
{   
    htable_t *hash_table = (htable_t *)calloc(1, sizeof(htable_t));
    if(!hash_table)
        return NULL;

    hash_table->table = (item_t**)calloc(size, sizeof(item_t*));
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
    if(ht_find_by_key_and_value(ht, key, value, NULL) != ENOKEY)
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



int ht_find_by_key_and_value(htable_t *ht, hkey_t key, hvalue_t *value, item_t **item)
{
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
    int index = ht->hash_func(key, HASH_CONST_2) % ht->size;
    
    if(!ht->table[index]) {
        return ENOKEY;
    } else if (ht->table[index]->key == key &&
            ht->table[index]->next == NULL) {
        free(ht->table[index]);
        ht->table[index] = NULL;
        return 0;
    } else if (ht->table[index]->key == key) {
        item_t *delptr = ht->table[index];
        ht->table[index] = ht->table[index]->next;
        ht->table[index]->prev = NULL;
        free(delptr);
        return 0;
    } else {
        item_t *ptr1 = ht->table[index]->next;
        item_t *ptr2 = ht->table[index];
        while(ptr1 != NULL && ptr1->key != key) {
            ptr2 = ptr1;
            ptr1 = ptr1->next;
        }
        if(ptr1 == NULL) {
            return ENOKEY;
        } else {
            ptr2->next = ptr1->next;
            free(ptr1);
            return 0;
        }
    }
}



int ht_del_item_by_key_and_value(htable_t *ht, hkey_t key, hvalue_t value)
{
    int index = ht->hash_func(key, HASH_CONST_2) % ht->size;
    
    if(!ht->table[index]) {
        return ENOKEY;
    } else if (ht->table[index]->key == key && ht->table[index]->value == value && ht->table[index]->next == NULL) {
        free(ht->table[index]);
        ht->table[index] = NULL;
        return 0;
    } else if(ht->table[index]->key == key && ht->table[index]->value == value) {
        item_t *delptr = ht->table[index];
        ht->table[index] = ht->table[index]->next;
        free(delptr);
        return 0;
    } else {
        item_t *ptr1 = ht->table[index]->next;
        item_t *ptr2 = ht->table[index];
        
        while(ptr1 != NULL && ptr1->key != key && ptr1->value != value) {
            ptr2 = ptr1;
            ptr1 = ptr1->next;
        }
        if(ptr1 == NULL)
            return ENOKEY;
        else {
            ptr2->next = ptr1->next;
            free(ptr1);
            return 0;
        }
    }
}

