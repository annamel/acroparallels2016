#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "hash_table.h"
#include "hash_table_debug.h"

#define _HASH_TABLE_FREE_ELEM(elem) do                                          \
                                        {                                       \
                                        free(elem->key);   elem->key    = NULL; \
                                        free(elem->value); elem->value  = NULL; \
                                        free(elem);        elem         = NULL; \
                                        } while (0);                            \

#define HASH_TABLE_ACT_ADD 0
#define HASH_TABLE_ACT_DEL 1

struct hash_table_elem
        {
        size_t key_s;
        size_t value_s;

        void * key;
        void * value;

        struct hash_table_elem * next;
        };

struct hash_table
        {
        struct hash_table_elem  ** entry;

        size_t count;
        size_t size;
        };

// Internal functions >>>

size_t _hash(hash_table_t * hash_table, void * key, size_t key_s);
ret_code_t _hash_table_find_in_list_and_act(hash_table_t * hash_table, size_t hash, hash_table_elem_t * new_elem, void * key, size_t key_s, int action);

//

ret_code_t hash_table_construct(hash_table_t ** hash_table_ptr, size_t size)
        {
        printf("SUCCESS");
        if (!hash_table_ptr || size <= 0)
                {
                return WRONG_ARGUMENTS;
                }

        hash_table_t * new_hash_table = (hash_table_t *)calloc(1, sizeof(hash_table_t));
        if (!new_hash_table)
                {
                DEBUG_LOG_ARGS("failed to allocate memory for new hash_table. Reason: %s", strerror(errno));
                goto error_hash_table_mem_alloc;
                }

        new_hash_table->size = size;
        sleep(1000);

        new_hash_table->entry = (hash_table_elem_t **) calloc(new_hash_table->size, sizeof(hash_table_elem_t *));
        if (!new_hash_table->entry)
                {
                DEBUG_LOG_ARGS("failed to allocate memory for new hash table entry. Reason: %s", strerror(errno));
                free(new_hash_table);
                goto error_hash_table_entry_mem_alloc;
                }

        *hash_table_ptr = new_hash_table;
        return SUCCESS;

        error_hash_table_entry_mem_alloc:
                free(new_hash_table);
                new_hash_table = NULL;

        error_hash_table_mem_alloc:

        return ERROR;
        }

ret_code_t hash_table_destruct(hash_table_t ** hash_table_ptr)
        {
        if (!hash_table_ptr)
                {
                return WRONG_ARGUMENTS;
                }

        hash_table_t * hash_table = *hash_table_ptr;

        int i = 0;
        for (i = 0; i < hash_table->size; i++)
                {
                while (hash_table->entry[i])
                        {
                        hash_table_elem_t * temp_elem = hash_table->entry[i];
                        hash_table->entry[i] = hash_table->entry[i]->next;

                        _HASH_TABLE_FREE_ELEM(temp_elem);
                        }
                }
        free(hash_table->entry);
        hash_table->entry = NULL;

        free(hash_table);
        hash_table = NULL;

        return SUCCESS;
        }

ret_code_t hash_table_put(hash_table_t * hash_table, void * key, size_t key_s, void * value, size_t value_s)
        {
        if (!hash_table || !key || !value || key_s <= 0 || value_s <= 0)
                {
                return WRONG_ARGUMENTS;
                }

        ret_code_t    ret     = 0;
        void *        ret_ptr = NULL;

        hash_table_elem_t * new_elem = (hash_table_elem_t *)calloc(1, sizeof(hash_table_elem_t));
        if (!new_elem)
                {
                DEBUG_LOG_ARGS("failed to allocate memory for new element. Reason: %s", strerror(errno));
                goto error_new_elem_mem_alloc;
                }

        new_elem->key = calloc(1, key_s);
        if (!new_elem->key)
                {
                DEBUG_LOG_ARGS("failed to allocate memory for key in new element. Reason: %s", strerror(errno));
                goto error_new_elem_key_mem_alloc;
                }

        new_elem->value = calloc(1, value_s);
        if (!new_elem->value)
                {
                DEBUG_LOG_ARGS("failed to allocate memory for value in new element. Reason: %s", strerror(errno));
                goto error_new_elem_value_mem_alloc;
                }

        ret_ptr = memcpy(new_elem->key, key, key_s);
        if (!ret_ptr)
                {
                DEBUG_LOG("failed to copy key to the new element");
                goto error_key_mem_copy;
                }

        ret_ptr = memcpy(new_elem->value, value, value_s);
        if (!ret_ptr)
                {
                DEBUG_LOG("failed to copy value to the new element");
                goto error_value_mem_copy;
                }

        new_elem->key_s   = key_s;
        new_elem->value_s = value_s;

        new_elem->next = NULL;

        size_t hash = _hash(hash_table, key, key_s);
        if (hash >= hash_table->size)
                {
                DEBUG_LOG_ARGS("invalid hash: %zu", hash);
                goto error_invalid_hash;
                }

        if (!hash_table->entry[hash])
                {
                hash_table->entry[hash] = new_elem;
                hash_table->count++;

                return SUCCESS;
                }

        ret = _hash_table_find_in_list_and_act(hash_table, hash, new_elem, key, key_s, HASH_TABLE_ACT_ADD);

        if (!ret)
                {
                return SUCCESS;
                }

        error_invalid_hash:

        error_value_mem_copy:

        error_key_mem_copy:
                free(new_elem->value);
                new_elem->value = NULL;

        error_new_elem_value_mem_alloc:
                free(new_elem->key);
                new_elem->key = NULL;

        error_new_elem_key_mem_alloc:
                free(new_elem);
                new_elem = NULL;

        error_new_elem_mem_alloc:

        return ERROR;
        }

ret_code_t hash_table_get(hash_table_t * hash_table, void * key, size_t key_s, void ** value_ptr, size_t value_s)
        {
        if (!hash_table || !key || key_s <= 0 || !value_ptr || value_s <= 0)
                {
                return WRONG_ARGUMENTS;
                }

        size_t hash = _hash(hash_table, key, key_s);
        if (hash >= hash_table->size)
                {
                DEBUG_LOG_ARGS("invalid hash: %zu", hash);
                return ERROR;
                }

        if (!hash_table->entry[hash])
                {
                DEBUG_LOG("element with such key was not found");
                return ERROR;
                }

        hash_table_elem_t * temp_elem = hash_table->entry[hash];
        while (temp_elem)
                {
                while (temp_elem && (temp_elem->key_s != key_s))
                        {
                        temp_elem = temp_elem->next;
                        }

                if (temp_elem)
                        {
                        if (!memcmp(temp_elem->key, key, key_s))
                                {

                                memcpy(value_ptr, &temp_elem->value, value_s);

                                return SUCCESS;
                                }
                        else
                                {
                                temp_elem = temp_elem->next;
                                }
                        }
                }

        return ERROR;
        }


ret_code_t hash_table_remove(hash_table_t * hash_table, void * key, size_t key_s)
        {
        if (!hash_table || !key || key_s <= 0)
                {
                return WRONG_ARGUMENTS;
                }

        size_t hash = _hash(hash_table, key, key_s);
        if (hash >= hash_table->size)
                {
                DEBUG_LOG_ARGS("invalid hash: %zu", hash);
                return ERROR;
                }

        if (!hash_table->entry[hash])
                {
                DEBUG_LOG("element with such key was not found");
                return ERROR;
                }

        ret_code_t ret = 0;
        ret = _hash_table_find_in_list_and_act(hash_table, hash, NULL, key, key_s, HASH_TABLE_ACT_DEL);
        if (!ret)
                {
                return SUCCESS;
                }

        DEBUG_LOG("element with such key was not found");
        return ERROR;
        }

void hash_table_print(hash_table_t * hash_table)
        {
        int i = 0;
        for (i = 0; i < hash_table->size; i++)
                {
                printf("> ");

                hash_table_elem_t * temp_elem = hash_table->entry[i];
                while(temp_elem)
                        {
                        printf("(%zu, %p) ", *(size_t *)temp_elem->key, *(void **)temp_elem->value);
                        temp_elem = temp_elem->next;
                        }
                printf("\n");
                }
        }

size_t _hash(hash_table_t * hash_table, void * key_ptr, size_t key_s) // Implement this function for your data type
        {
        size_t key = *(size_t *)key_ptr;
        return (size_t)(key % hash_table->size);
        }

ret_code_t _hash_table_find_in_list_and_act(hash_table_t * hash_table, size_t hash, hash_table_elem_t * new_elem, void * key, size_t key_s, int action)
        {
        hash_table_elem_t * temp_elem = hash_table->entry[hash];
        hash_table_elem_t * temp_prev_elem = NULL;
        while (temp_elem)
                {
                while (temp_elem && (temp_elem->key_s != key_s))
                        {
                        temp_prev_elem = temp_elem;
                        temp_elem = temp_elem->next;
                        }

                if (temp_elem)
                        {
                        if (!memcmp(temp_elem->key, key, key_s))
                                {
                                hash_table_elem_t * to_delete_elem = temp_elem;

                                if (!temp_prev_elem)
                                        {
                                        if (action == HASH_TABLE_ACT_ADD)
                                                {
                                                hash_table->entry[hash] = new_elem;
                                                }
                                        else
                                                {
                                                hash_table->entry[hash] = temp_elem->next;
                                                }
                                        }
                                else
                                        {
                                        if (action == HASH_TABLE_ACT_ADD)
                                                {
                                                temp_prev_elem->next = new_elem;
                                                }
                                        else
                                                {
                                                temp_prev_elem->next = temp_elem->next;
                                                }
                                        }

                                if (action == HASH_TABLE_ACT_ADD)
                                        {
                                        new_elem->next = to_delete_elem->next;
                                        }

                                _HASH_TABLE_FREE_ELEM(to_delete_elem);

                                if (action == HASH_TABLE_ACT_DEL)
                                        {
                                        hash_table->count--;
                                        }

                                return SUCCESS;
                                }
                        else
                                {
                                temp_prev_elem = temp_elem;
                                temp_elem = temp_elem->next;
                                }
                        }
                }
        if (action == HASH_TABLE_ACT_ADD)
                {
                temp_prev_elem->next = new_elem;
                hash_table->count++;
                }

        return SUCCESS;
        }
