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
    ss_item_t *next_index;
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


    //Case 1: adding in the head
    if(sset->size == 0
      || (sset->head && chunk->index < sset->head->chunk->index)
      || (sset->head && chunk->index == sset->head->chunk->index &&
          chunk->len > sset->head->chunk->len) )
    {
        item->next = sset->head;
        if(sset->size)
            if(chunk->index == sset->head->chunk->index)
                item->next_index = sset->head->next_index;
            else
                item->next_index = sset->head;
        sset->head = item;
    } //Case 2: adding not in the head
    else
    {
        ss_item_t *sitem = sset->head;

        //Case 2.1: adding into the head index group
        if(chunk->index == sset->head->chunk->index)
        {
            while(sitem->next && sitem->next->chunk->len > chunk->len
                  && sitem->next->chunk->index == chunk->index)
                sitem = sitem->next;

            if( (sitem->chunk->index == chunk->index &&
                 sitem->chunk->len == chunk->len) ||
                (sitem->next && sitem->next->chunk->len == chunk->len &&
                 sitem->next->chunk->index == chunk->index) )
            {
                log_write(WARNING, "ss_add: trying to add existing chunk");
                log_write(INFO,"ss_add: finished");
                return EKEYREJECTED;
            }            
        } //Case 2.2: adding not into the head index group(chunk index
        else        //more than head index)
        {
            while(sitem->next_index && sitem->next_index->chunk->index < chunk->index)
                sitem = sitem->next_index;

            ss_item_t *prev_idx_gr = sitem; // For remember previous index group
            //Case 2.2.1: adding into the tail
            if(!sitem->next_index)
                while(sitem->next)
                {
                    sitem->next_index = item;
                    sitem = sitem->next;
                }
            else if(sitem->next_index->chunk->index == chunk->index) //Case 2.2.2: adding into
            {                                                        //the finded group
                if(sitem->next_index->chunk->len < chunk->len)
                    while(sitem->next->chunk->index == sitem->chunk->index)
                    {
                        sitem->next_index = item;
                        sitem = sitem->next;
                    }
                else if(sitem->next_index->chunk->len == chunk->len)
                {
                    log_write(WARNING, "ss_add: trying to add existing chunk");
                    log_write(INFO,"ss_add: finished");
                    return EKEYREJECTED;
                }
                else
                {
                    sitem = sitem->next_index;
                    while(sitem->next->chunk->index == chunk->index &&
                          sitem->next->chunk->len > chunk->len)
                        sitem = sitem->next;

                    if(sitem->next->chunk->len == chunk->len &&
                       sitem->next->chunk->index == chunk->index)
                    {
                        log_write(WARNING, "ss_add: trying to add existing chunk");
                        log_write(INFO,"ss_add: finished");
                        return EKEYREJECTED;
                    }
                }
            }
            else //Case 2.2.3: adding between two index
            {    //groups(after current sitem index group)
                if(sitem->next->chunk->index < chunk->index)
                    while(sitem->next->chunk->index < chunk->index)
                    {
                        sitem->next_index = item;
                        sitem = sitem->next;
                    }
            }
        }

        //  Adding
        item->next = sitem->next;
        if(sitem->next && sitem->next_index != sitem->next->next_index
                && item->chunk->index == sitem->next_index->chunk->index)
            item->next_index = sitem->next->next_index;
        else
            item->next_index = sitem->next_index;
        sitem->next = item;
        if(sitem->chunk->index < item->chunk->index)
            sitem->next_index = item;
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
    int error = 0;


    ss_item_t *sitem = sset->head;

    //Case 1: index less than any in the set, set is empty or
    // set contains 1 element which is not to delete
    if( (sset->size == 0) || (index < sitem->chunk->index) ||
        (!sitem->next && (sitem->chunk->index != index
                       || sitem->chunk->len != len) ) )
    {
        error = ENOKEY;
        goto end;
    } //Case 2: delete from head index group
    else if(sitem->chunk->index == index)
    {
        if(sitem->chunk->len == len) //Case 2.1: delete from head
        {
            sset->head = sitem->next;
            free(sitem);
            sitem = NULL;
            goto end;
        } //Find deleting chunk in the head index group
        else if(sitem->chunk->len > len)
            while(sitem->next && sitem->next->chunk->len > len &&
                  sitem->next->chunk->index == index)
                sitem = sitem->next;

        //Case 2.2: deleting chunk was not finded
        if(!sitem ||
           !sitem->next ||
            sitem->chunk->len < len ||
            sitem->next->chunk->len < len ||
            sitem->next->chunk->index != index)
        {
            error = ENOKEY;
            goto end;
        } //Case 2.3: finded
        else if(sitem->next->chunk->len == len)
        {
            ss_item_t *del_item = sitem->next;
            sitem->next = sitem->next->next;
            free(del_item);
            del_item = NULL;
            goto end;
        }
    } //Case 3: delete from not head index group
    else
    {
        while(sitem->next_index && sitem->next_index->chunk->index < index)
            sitem = sitem->next_index;

        //Case 3.1: deleting chunk was not finded in the first position
        if(!sitem->next_index ||
            sitem->next_index->chunk->index > index ||
            (sitem->next_index->chunk->index == index
             && sitem->next_index->chunk->len < len) )
        {
            error = ENOKEY;
            goto end;
        }

        //Case 3.2: delete from finded index group
        if(sitem->next_index->chunk->len == len) //Case 3.2.1: chunk
        {                                        //first in the group
            while(sitem->next != sitem->next_index)
            {
                sitem->next_index = sitem->next_index->next;
                sitem = sitem->next;
            }

            ss_item_t *del_item = sitem->next;
            sitem->next = sitem->next->next;
            sitem->next_index = sitem->next_index->next;
            free(del_item);
            del_item = NULL;
            goto end;
        }
        else //Case 3.2.2: chunk not first in the group
        {
            sitem = sitem->next_index;
            while(sitem->next &&
                  sitem->next->chunk->len > len &&
                  sitem->next->chunk->index == index)
                sitem = sitem->next;

            //Case 3.2.2.1: chunk doesn't exist
            if(!sitem->next ||
                 sitem->next->chunk->index != index ||
                (sitem->next->chunk->index == index
                 && sitem->next->chunk->len < len) )
            {
                error = ENOKEY;
                goto end;
            }
            else //Case 3.2.2.2: chunk has find
            {
                ss_item_t *del_item = sitem->next;
                sitem->next = sitem->next->next;
                free(del_item);
                del_item = NULL;
                goto end;
            }
        }
    }

end:
    if(error)
        log_write(INFO, "ss_del: trying to delete nonexistent item");
    else
        sset->size --;

    log_write(INFO, "ss_del: finished");
    return error;
}



/*int ss_find(sset_t *sset, off_t index, off_t len, chunk_t **chunk)
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
}*/



int ss_find(sset_t *sset, off_t index, off_t len, chunk_t **chunk)
{
    if(!sset)
        return EINVAL;
    log_write(INFO, "ss_find: started");
    int error = 0;


    ss_item_t *sitem = sset->head;
    while(sitem &&
          sitem->chunk->index < index &&
          (index - sitem->chunk->index + len) > sitem->chunk->len)
        sitem = sitem->next_index;


    if(!sitem || sitem->chunk->index > index)
        error = ENOKEY;


    if(error)
        log_write(INFO, "ss_del: trying to delete nonexistent item");
    else
        *chunk = sitem->chunk;

    log_write(INFO, "ss_del: finished");
    return error;
}



int ss_print(sset_t *sset, unsigned int from, unsigned int to)
{
    if(!sset)
        return EINVAL;
    log_write(INFO, "ss_print: started");

    printf("\n**********************************************");
    printf("\n***************   SORTED SET   ***************");
    printf("\n**********************************************\n");

    if(sset->size == 0)
        printf("set is empty\n");
    else
    {
        int i = 0;
        for(ss_item_t *item = sset->head; item; item = item->next)
            if(i >= from && i < to)
                printf("[%d]: index=%d, len=%d, (self: %u,next: %u,next index: %u)\n", i++,
                item->chunk->index, item->chunk->len, item, item->next, item->next_index);

        printf("***set size: %d\n", sset->size);
        printf("***head: %d\n", sset->head);
    }

    printf("**********************************************\n");
    log_write(INFO, "ss_print: finished");
    return 0;
}
