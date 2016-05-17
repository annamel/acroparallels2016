#ifndef __LIST_H_INCLUDED
#define __LIST_H_INCLUDED

#include <stdio.h>      // just for 'NULL'

#define offsetof(type, member) ((size_t) &((type *)0)->member)

/**
  * container_of - cast a member of a structure out to the containing structure
  * @ptr:        the pointer to the member.
  * @type:       the type of the container struct this is embedded in.
  * @member:     the name of the member within the struct.
  *
  */
#define container_of(ptr, type, member)                                 \
                ({                                                      \
                const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
                (type *)( (char *)__mptr - offsetof(type,member) );     \
                })

//==============================================================================
// Declaring of a structure
//==============================================================================
typedef struct list_node list_node_t;
struct list_node
        {
        list_node_t* prev;
        list_node_t* next;
        };
//==============================================================================

//==============================================================================
// Realisation of functions
//==============================================================================
static inline void _list_head_init (list_node_t* head)
        {
        head->prev = head;
        head->next = head;
        }

// add a new element between two old one
static inline void _list_add (list_node_t* new,
                              list_node_t* prev,
                              list_node_t* next)
        {
        prev->next = new;
        new->prev = prev;
        new->next = next;
        next->prev = new;
        }

// add a new element before the specified head (like to the tail of list)
static inline void _list_add_tail (list_node_t* new, list_node_t* head)
        {
        _list_add (new, head->prev, head);
        }

static inline void _list_del (list_node_t* prev, list_node_t* next)
        {
        prev->next = next;
        next->prev = prev;
        }

static inline void _list_del_entry (list_node_t* entry)
        {
        _list_del (entry->prev, entry->next);
        entry->prev = NULL;
        entry->next = NULL;
        }
//==============================================================================


#endif // __LIST_H_INCLUDED

