#include "sorted_set.h"
#include <errno.h>
#include <malloc.h>
#include "../logger/logger.h"



struct sorted_set
{
    unsigned int size;
    ss_item_t *head;
};



struct sset_item
{
    ss_item_t *next;
    chunk_t *chunk;
};




sset_t *ss_init()
{
    log_write(INFO, "ss_init: started");

    sset_t *sset = (sset_t *)calloc(1, sizeof(sset_t));
    if(!sset)
    {
        log_write(ERROR, "ss_init: can't allocate memory for new sset");
        return NULL;
    }

    sset->head = NULL;
    sset->size = 0;

    log_write(INFO, "ss_init: finished");
    return sset;
}



int ss_deinit(sset_t *sset)
{
    if(!sset)
        return EINVAL;
    log_write(INFO, "ss_deinit: started");

    int error = 0;
    while(!error && sset->size)
        error = ss_del(sset, sset->head->chunk->index,
                             sset->head->chunk->len);

    free(sset);

    log_write(INFO, "ss_deinit: finished");
    return 0;
}



int ss_add(sset_t *sset, chunk_t *chunk)
{
    if(!sset || !chunk)
        return EINVAL;
    log_write(INFO, "ss_add: started");


    ss_item_t *item = (ss_item_t *)calloc(1, sizeof(ss_item_t));
    if(!item)
    {
        log_write(ERROR, "ss_add: can't allocate memory for new item");
        return -1;
    }
    item->chunk = chunk;


    ss_item_t *curr_item = sset->head;
    if(sset->size == 0) // If set is empty
    {
        item->next = NULL;
        sset->head = item;
    }
    else if(curr_item->chunk->index > chunk->index) // If adding value less
    {                                               // than any value in set
        item->next = sset->head;
        sset->head = item;
    }
    else if(curr_item->chunk->index == chunk->index) // If adding value index equals
    {                                                // head value index of set
        if(curr_item->chunk->len > chunk->len)
        {
            while(curr_item->next && curr_item->chunk->len > chunk->len)
                curr_item = curr_item->next;

            if(curr_item->chunk->len == chunk->len)
            {
                log_write(INFO, "ss_add: trying to add existing item");
                log_write(INFO, "ss_add: finished");
                return EKEYREJECTED;
            }
            item->next = curr_item->next;
            curr_item->next = item;
        }
        else
        {
            sset->head = item;
            item->next = curr_item;
        }

    }
    else // Other ways
    {
        while(curr_item->next && curr_item->next->chunk->index < chunk->index)
            curr_item = curr_item->next;

        if(curr_item->next && curr_item->next->chunk->index == chunk->index)
            while(curr_item->next && curr_item->chunk->len > chunk->len)
                curr_item = curr_item->next;

        if(curr_item->next && curr_item->next->chunk->index == chunk->index
                           && curr_item->next->chunk->len == chunk->len)
        {
            log_write(INFO, "ss_add: trying to add existing item");
            log_write(INFO, "ss_add: finished");
            return EKEYREJECTED;
        }

        item->next = curr_item->next;
        curr_item->next = item;
    }


    sset->size++;
    log_write(INFO, "ss_add: finished");
    return 0;
}



int ss_del(sset_t *sset, off_t index, off_t len)
{
    if(!sset)
        return EINVAL;
    log_write(INFO, "ss_del: started");

    ss_item_t *curr_item = sset->head;
    if((sset->size == 0) || (index < curr_item->chunk->index) ||
       (!curr_item->next && curr_item->chunk->index != index
                         && curr_item->chunk->len != len))
    {
        log_write(INFO, "ss_del: trying to delete nonexistent item");
        log_write(INFO, "ss_del: finished");
        return ENOKEY;
    }

    if(curr_item->chunk->index == index &&
            curr_item->chunk->len == len)
    {
        sset->head = curr_item->next;
        free(curr_item);
    }
    else
    {
        while(curr_item->next &&
              curr_item->next->chunk->index < index)
            curr_item = curr_item->next;

        if(curr_item->next->chunk->index == index)
            while(curr_item->next && curr_item->next->chunk->len > len
               && curr_item->next->chunk->index == index)
                curr_item = curr_item->next;

        if(!curr_item->next || curr_item->next->chunk->len != len)
        {
            log_write(INFO, "ss_del: trying to delete nonexistent item");
            log_write(INFO, "ss_del: finished");
            return ENOKEY;
        }

        ss_item_t *del_ptr = curr_item->next;
        curr_item->next = del_ptr->next;

        free(del_ptr);
    }

    sset->size --;
    log_write(INFO, "ss_del: finished");
    return 0;
}



int ss_find(sset_t *sset, off_t index, off_t len, chunk_t **chunk)
{
    if(!sset)
        return EINVAL;
    log_write(INFO, "ss_find: started");

    ss_item_t *curr_item = sset->head;
    while(curr_item && curr_item->chunk->index <= index &&
          (index - curr_item->chunk->index + len) > curr_item->chunk->len)
        curr_item = curr_item->next;        

    if(!curr_item || curr_item->chunk->index > index)
    {
        log_write(INFO, "ss_del: don't' find needed item");
        log_write(INFO, "ss_del: finished");
        return ENOKEY;
    }

    *chunk = curr_item->chunk;
    log_write(INFO, "ss_find: finished");
    return 0;
}



int ss_print(sset_t *sset)
{
    if(!sset)
        return EINVAL;
    log_write(INFO, "ss_print: started");

    printf("\n**************************");
    printf("\n*****   SORTED SET   *****");
    printf("\n**************************\n");

    if(sset->size == 0)
        printf("set is empty");
    else
    {
        int i = 0;
        for(ss_item_t *item = sset->head; item; item = item->next)
            printf("[%d]: index=%d, len=%d, (self: %u,next: %u)\n", i++,
            item->chunk->index, item->chunk->len, item, item->next);
        printf("***set size: %d\n", sset->size);
        printf("***head: %d\n", sset->head);
    }

    printf("**************************\n");
    log_write(INFO, "ss_print: finished");
    return 0;
}
