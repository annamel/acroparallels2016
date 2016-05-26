#include "skiplist.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>



#define SKIPLIST_MAX_LEVEL 6



skiplist_t *skiplist_init()
{
    skiplist_t *list = (skiplist_t *)calloc(1, sizeof(skiplist_t));
    if(!list)
        return ENOMEM;

    snode_t *header = (snode_t *)malloc(sizeof(snode_t));
    list->header = header;
    if(!header)
    {
        free(list);
        return ENOMEM;
    }

    header->key = INT_MAX;
    header->forward = (snode_t **)calloc(SKIPLIST_MAX_LEVEL+1, sizeof(snode_t *));
    if(!header->forward)
    {
        free(header);
        free(list);
        return ENOMEM;
    }

    for (int i = 0; i <= SKIPLIST_MAX_LEVEL; i++) {
        header->forward[i] = list->header;
    }

    list->level = 1;
    list->size = 0;

    return list;
}



static int rand_level()
{
    int level = 1;
    while (rand() < RAND_MAX/2 && level < SKIPLIST_MAX_LEVEL)
        level++;

    return level;
}



int skiplist_add(skiplist_t *list, slkey_t key, slvalue_t value)
{
    if(!list || !value)
        return EINVAL;

    snode_t *update[SKIPLIST_MAX_LEVEL+1];
    snode_t *x = list->header;
    int level = 0;
    for (int i = list->level; i >= 1; i--)
    {
        while (x->forward[i]->key < key)
            x = x->forward[i];
        update[i] = x;
    }
    x = x->forward[1];

    if (key == x->key)
    {
        x->value = value;
        return 0;
    }
    else
    {
        level = rand_level();
        if (level > list->level)
        {
            for (int i = list->level+1; i <= level; i++)
            {
                update[i] = list->header;
            }
            list->level = level;
        }

        x = (snode_t *)calloc(1, sizeof(snode_t));
        if(!x)
            return ENOMEM;

        x->key = key;
        x->value = value;
        x->forward = (snode_t **)calloc(level+1, sizeof(snode_t *));
        for (int i = 1; i <= level; i++)
        {
            x->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = x;
        }
    }

    return 0;
}



int skiplist_find(skiplist_t *list, off_t index, off_t len, chunk_t **chunk)
{
    if(!list)
        return EINVAL;

    snode_t *x = list->header;
    for (int i = list->level; i >= 1; i--)
        while (x->forward[i]->key < index)
            x = x->forward[i];

    if (x->forward[1]->key == index)
        *chunk = (chunk_t *)(x->forward[1]->value);
    else
        return ENOKEY;

    return 0;
}



static void skiplist_node_free(snode_t *x)
{
    if (x)
    {
        free(x->forward);
        free(x);
    }
}



int skiplist_del(skiplist_t *list, slkey_t key)
{
    if(!list)
        return EINVAL;

    snode_t *update[SKIPLIST_MAX_LEVEL + 1];
    snode_t *x = list->header;
    for (int i = list->level; i >= 1; i--)
    {
        while (x->forward[i]->key < key)
            x = x->forward[i];
        update[i] = x;
    }

    x = x->forward[1];
    if (x->key == key)
    {
        for (int i = 1; i <= list->level; i++)
        {
            if (update[i]->forward[i] != x)
                break;
            update[i]->forward[i] = x->forward[i];
        }
        skiplist_node_free(x);

        while (list->level > 1 && list->header->forward[list->level] == list->header)
            list->level--;
        return 0;
    }

    return ENOKEY;
}



void skiplist_dump(skiplist_t *list)
{
    snode_t *x = list->header;
    while (x && x->forward[1] != list->header)
    {
        printf("%d[%d]->", x->forward[1]->key, x->forward[1]->value);
        x = x->forward[1];
    }
    printf("NIL\n");
}



int skiplist_deinit(skiplist_t *list)
{
    snode_t *current_node = list->header->forward[1];
    while(current_node != list->header)
    {
        snode_t *next_node = current_node->forward[1];
        free(current_node->forward);
        free(current_node);
        current_node = next_node;
    }

    free(list->header->forward);
    free(list->header);
    free(list);

    return 0;
}
