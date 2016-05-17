#include "hash_table.h"
#include "list.h"
#include "error.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>


// if (1) - enables inner checkings
// like what args passed to internal '_***' funcs, and so on
#define IS_DEBUG_ON     0

//---------------------------
// for testing purposes only
//---------------------------
#define GCOV_MEM_ERR_ON 0

#if (GCOV_MEM_ERR_ON == 1)
        #include "my_mem_funcs.h"
        #define calloc my_calloc
        #define memcpy my_memcpy
#else
        #include <malloc.h>
#endif
//---------------------------


//==============================================================================
// Declaring of structures
//==============================================================================
typedef struct Hash_elem hash_elem_t;
struct Hash_elem
        {
        list_node_t   list_node;

        hash_key_t*   key;
        size_t        key_size;
        hash_entry_t* entry;
        size_t        entry_size;
        };

typedef struct Hash_table
        {
        size_t        size;
        hash_func_t   func;
        list_node_t** idx_arr;
        } hash_table_t;

typedef struct Iterator
        {
        const hash_table_t* hash_table;
        list_node_t*        list_node;
        } iterator_t;
//==============================================================================


//==============================================================================
// Prototypes of internal functions
//==============================================================================
static hash_elem_t* _create_hash_elem (const hash_key_t* hash_key, size_t hash_key_size,
                                       const hash_entry_t* entry_to_add, size_t entry_to_add_size);
static inline int _delete_hash_elem_from_mem (hash_elem_t** hash_elem);

static inline iterator_t* _create_iterator (const hash_table_t* hash_table, const list_node_t* list_node);
//==============================================================================
// Realisation of functions
//==============================================================================

// only for internal usage
// RETURN VALUE: hash_elem_t* - if success
//               NULL         - otherwise, with my_errno indicating the error
static hash_elem_t* _create_hash_elem (const hash_key_t* hash_key, size_t hash_key_size,
                                       const hash_entry_t* entry_to_add, size_t entry_to_add_size)
{
hash_elem_t* hash_elem = NULL;

#if (IS_DEBUG_ON == 1)
if ((hash_key && hash_key_size && entry_to_add && entry_to_add_size) == 0)
        {
        my_errno = wrong_args;
        return NULL;
        }
#endif
//-----
// alloc only one large piece of memory and place key and entry of hash_elem,
//      after hash_elem structure itself in a row
hash_elem = (hash_elem_t*) calloc (1, sizeof (hash_elem_t) + hash_key_size + entry_to_add_size);
if (hash_elem == NULL)
        goto error_hash_elem;
//-----
#if (IS_DEBUG_ON == 1)
        hash_elem->list_node.prev = NULL;
        hash_elem->list_node.next = NULL;
#endif
//-----
hash_elem->key = (hash_key_t*)(hash_elem + 1);
if (memcpy (hash_elem->key, hash_key, hash_key_size) == NULL)
        goto error_memcpy_key;

hash_elem->key_size = hash_key_size;
//-----
hash_elem->entry = (hash_entry_t*)((char*)hash_elem->key + hash_key_size);
if (memcpy (hash_elem->entry, entry_to_add, entry_to_add_size) == NULL)
        goto error_memcpy_entry;

hash_elem->entry_size = entry_to_add_size;
//-----
return hash_elem;


error_memcpy_entry:
error_memcpy_key:
        free (hash_elem);
        #if (IS_DEBUG_ON == 1)
        hash_elem = NULL;
        #endif
error_hash_elem:
        my_errno = mem_err;
        return NULL;
}

// only for internal usage
// deletes only memory reserved by current hash_elem structure,
//      but do not delete it from hash_table lists properly
// RETURN VALUE:  0 - if success
//               -1 - otherwise, with my_errno indicating the error (if wrong args was given actually)
static inline int _delete_hash_elem_from_mem (hash_elem_t** hash_elem)
{
#if (IS_DEBUG_ON == 1)
if (hash_elem == NULL)
        {
        my_errno = wrong_args;
        return -1;
        }
if (*hash_elem == NULL)
        {
        my_errno = wrong_args;
        return -1;
        }
#endif

free (*hash_elem); *hash_elem = NULL;

return 0;
}

// RETURN VALUE:  0 - if success
//               -1 - otherwise, with my_errno indicating the error
int Hash_table_construct (hash_table_t** hash_table, size_t hash_table_size, const hash_func_t hashing_func)
{
// TODO:
hash_table_t* hash_table_ptr = NULL;

if ((hash_table && hash_table_size && hashing_func) == 0)
        {
        my_errno = wrong_args;
        return -1;
        }
//-----
// allocate memory for the array and for hash_table_t as a head for this array
hash_table_ptr = (hash_table_t*) calloc (1, sizeof (hash_table_t) + hash_table_size * sizeof (list_node_t*));
if (hash_table_ptr == NULL)
        {
        my_errno = mem_err;
        return -1;
        }
//-----
hash_table_ptr->size    = hash_table_size;
hash_table_ptr->func    = hashing_func;
hash_table_ptr->idx_arr = (list_node_t**)(hash_table_ptr + 1);
//-----
*hash_table = hash_table_ptr;

return 0;
}

// RETURN VALUE:  0 - if success
//               -1 - otherwise, with my_errno indicating the error (if wrong args was given actually)
int Hash_table_destruct (hash_table_t** hash_table)
{
int i = 0;
list_node_t *head = NULL, *curr = NULL ;
hash_elem_t* curr_entry = NULL;

if (hash_table == NULL)
        goto error_wrong_args;
if (*hash_table == NULL)
        goto error_wrong_args;

// clear lists of hash_elem_t*
for (i = 0; i < (*hash_table)->size; i++)
        {
        head = (*hash_table)->idx_arr[i];

        if (head == NULL)
                continue;

        while ( !((head->next == head) && (head->prev == head)) )
                {
                curr = head->next;
                _list_del_entry (curr);

                curr_entry = container_of(curr, hash_elem_t, list_node);
                #if (IS_DEBUG_ON == 1)
                if (_delete_hash_elem_from_mem (&curr_entry))
                        assert (!"internal error in Hash_table_destruct: wrong args passed to _delete... func");
                #else
                _delete_hash_elem_from_mem (&curr_entry);
                #endif
                }

        free (head);
        #if (IS_DEBUG_ON == 1)
        head = NULL;
        #endif
        }

// destruct hash_table (and idx_array too)
free (*hash_table); *hash_table = NULL;

return 0;


error_wrong_args:
        my_errno = wrong_args;
        return -1;
}

int Hash_table_add_elem (hash_table_t* hash_table, const hash_key_t* hash_key, size_t hash_key_size,
                                                   const hash_entry_t* entry_to_add, size_t entry_to_add_size)
{
size_t idx_for_arr = 0;
hash_elem_t* hash_elem = NULL;
list_node_t* list_head = NULL;

if ((hash_table && hash_key && hash_key_size && entry_to_add && entry_to_add_size) == 0)
        {
        my_errno = wrong_args;
        return -1;
        }

idx_for_arr = hash_table->func (hash_key, hash_key_size, hash_table->size);
if (idx_for_arr >= hash_table->size)
        {
        my_errno = wrong_hash_func;
        return -1;
        }

hash_elem = _create_hash_elem (hash_key, hash_key_size, entry_to_add, entry_to_add_size);
if (hash_elem == NULL)
        return -1;      // my_errno was set in _create_hash_elem func

list_head = hash_table->idx_arr[idx_for_arr];
if (list_head == NULL)       // if there is no head of list
        {
        list_head = (list_node_t*) calloc (1, sizeof (list_node_t));
        if (list_head == NULL)
                goto error_list;

        _list_head_init (list_head);

        hash_table->idx_arr[idx_for_arr] = list_head;
        }

// add to the end of list
_list_add_tail (&(hash_elem->list_node), list_head);

return 0;


error_list:
        #if (IS_DEBUG_ON == 1)
        if (_delete_hash_elem_from_mem (&hash_elem))
                assert (!"internal error in Hash_table_add_elem: wrong args passed to _delete... func");
        #else
        _delete_hash_elem_from_mem (&hash_elem);
        #endif
        my_errno = mem_err;
        return -1;
}

int Hash_table_delete_elem (iterator_t** ptr_iter_to_del_elem)
{
iterator_t* list_iter = NULL;
hash_elem_t* elem_to_del = NULL;

if (ptr_iter_to_del_elem == NULL)
        goto error_wrong_args;
if (*ptr_iter_to_del_elem == NULL)
        goto error_wrong_args;

list_iter = *ptr_iter_to_del_elem;
#if (IS_DEBUG_ON == 1)
if (list_iter->hash_table == NULL)
        assert (!"internal error in Hash_table_delete_elem: list_iter->hash_table == NULL");
if (list_iter->list_node == NULL)
        assert (!"internal error in Hash_table_delete_elem: list_iter->list_node == NULL");
#endif

_list_del_entry (list_iter->list_node);
elem_to_del = container_of (list_iter->list_node, hash_elem_t, list_node);
#if (IS_DEBUG_ON == 1)
if (_delete_hash_elem_from_mem (&elem_to_del))
        assert (!"internal error in Hash_table_delete_elem: wrong args passed to _delete... func");
#else
_delete_hash_elem_from_mem (&elem_to_del);
#endif

#if (IS_DEBUG_ON == 1)
if (delete_iterator (&list_iter)) // as func returns -1 only if wrong args were passed
        assert (!"internal error in Hash_table_delete_elem: wrong args passed to delete_iterator func");
#else
delete_iterator (&list_iter);
#endif
*ptr_iter_to_del_elem = NULL;

return 0;


error_wrong_args:
        my_errno = wrong_args;
        return -1;
}

iterator_t* find_elem (const hash_table_ptr hash_table, const hash_key_t* hash_key, size_t hash_key_size)
{
size_t idx_for_arr = 0;
list_node_t *list_head = NULL, *list_iter = NULL;
hash_elem_t* hash_elem = NULL;
iterator_t* iter = NULL;

if ((hash_table && hash_key && hash_key_size) == 0)
        {
        my_errno = wrong_args;
        return NULL;
        }

idx_for_arr = hash_table->func (hash_key, hash_key_size, hash_table->size);
if (idx_for_arr >= hash_table->size)
        {
        my_errno = wrong_hash_func;
        return NULL;
        }

list_head = hash_table->idx_arr[idx_for_arr];
if ((list_head == NULL) || ((list_head->prev == list_head) && (list_head->next == list_head)))
        {
        my_errno = entry_not_found;
        return NULL;
        }

for (list_iter = list_head->next; list_iter != list_head; list_iter = list_iter->next)
        {
        hash_elem = container_of(list_iter, hash_elem_t, list_node);
        if (hash_elem->key_size != hash_key_size)
                continue;
        if (! memcmp (hash_elem->key, hash_key, hash_key_size))
                {
                // if we came here - we have found the elem that user asks for
                iter = _create_iterator (hash_table, list_iter);

                // if smth is wrong my_errno was set in _create_iterator func and iter == NULL
                return iter;
                }
        }

// if we came here - we haven't found the elem that user asks for
my_errno = entry_not_found;
return NULL;
}

//==============================================================================
// Iterators' functions
//==============================================================================

// only for internal usage
// RETURN VALUE: iterator_t* - if success
//               NULL        - otherwise, with my_errno indicating the error
static inline iterator_t* _create_iterator (const hash_table_t* hash_table, const list_node_t* list_node)
{
iterator_t* iter = NULL;

// I don`t check arguments, cause nothing bad will happen if they will be, for example, NULL;
//      maybe I'm  passing NULLs here, because want to fill the fields of iterator_t structure
//      later by hands.

iter = (iterator_t*) calloc (1, sizeof (iterator_t));
if (iter == NULL)
        {
        my_errno = mem_err;
        return NULL;
        }

iter->hash_table = hash_table;
iter->list_node  = (list_node_t*)list_node; // to avoid warnings about 'const'

return iter;
}

// RETURN VALUE: iterator_ptr - if success
//               NULL         - otherwise, with my_errno indicating the error
iterator_t* get_iterator (const hash_table_ptr hash_table)
{
size_t i = 0;
iterator_t*  iter = NULL;
list_node_t* list_head = NULL;

if (hash_table == NULL)
        {
        my_errno = wrong_args;
        return NULL;
        }

i = 0;
list_head = hash_table->idx_arr[0];
while ( (list_head == NULL) || ((list_head->prev == list_head) && (list_head->next == list_head)) )
        {
        i = ((i + 1) % (hash_table->size));
        list_head = hash_table->idx_arr[i];
        if (list_head == (hash_table->idx_arr[0]))
                {
                my_errno = table_empty;
                return NULL;
                }
        }

iter = _create_iterator (hash_table, list_head->next);
// if smth is wrong my_errno was set in _create_iterator func and iter == NULL

return iter;
}

int delete_iterator (iterator_ptr* usr_iter)
{
if (usr_iter == NULL)
        goto error_wrong_args;
if (*usr_iter == NULL)
        goto error_wrong_args;

#if (IS_DEBUG_ON == 1)
if ((*usr_iter)->hash_table == NULL)
        assert (!"internal error in delete_iterator: (*usr_iter)->hash_table == NULL");
if ((*usr_iter)->list_node == NULL)
        assert (!"internal error in delete_iterator: (*usr_iter)->list_node == NULL");
#endif

free (*usr_iter); *usr_iter = NULL;

return 0;


error_wrong_args:
        my_errno = wrong_args;
        return -1;
}

iterator_t* dup_iterator (const iterator_ptr usr_iter)
{
iterator_t* new_iter = NULL;

if (usr_iter == NULL)
        {
        my_errno = wrong_args;
        return NULL;
        }

#if (IS_DEBUG_ON == 1)
if (usr_iter->hash_table == NULL)
        assert (!"internal error in dup_iterator: usr_iter->hash_table == NULL");
if (usr_iter->list_node == NULL)
        assert (!"internal error in dup_iterator: usr_iter->list_node == NULL");
#endif

new_iter = (iterator_t*) calloc (1, sizeof (iterator_t));
if (new_iter == NULL)
        {
        my_errno = mem_err;
        return NULL;
        }

new_iter->hash_table = usr_iter->hash_table;
new_iter->list_node  = usr_iter->list_node;

return new_iter;
}

iterator_t* move_next (iterator_ptr usr_iter)
{
size_t idx_for_arr = 0;
list_node_t*  curr_list_node = NULL;
hash_elem_t*  curr_elem      = NULL;
hash_table_t* curr_table     = NULL;

if (usr_iter == NULL)
        {
        my_errno = wrong_args;
        return NULL;
        }

#if (IS_DEBUG_ON == 1)
if (usr_iter->hash_table == NULL)
        assert (!"internal error in move_next: usr_iter->hash_table == NULL");
if (usr_iter->list_node == NULL)
        assert (!"internal error in move_next: usr_iter->list_node == NULL");
#endif

curr_list_node = usr_iter->list_node;
curr_elem      = container_of (usr_iter->list_node, hash_elem_t, list_node);
curr_table     = (hash_table_t*)usr_iter->hash_table;

idx_for_arr = curr_table->func (curr_elem->key, curr_elem->key_size, curr_table->size);
if (idx_for_arr >= curr_table->size)
        {
        my_errno = wrong_hash_func;
        return NULL;
        }

if (curr_list_node->next != curr_table->idx_arr[idx_for_arr])
        {
        usr_iter->list_node = curr_list_node->next;
        return usr_iter;
        }

do
        {
        idx_for_arr = ((idx_for_arr + 1) % (curr_table->size));
        curr_list_node = curr_table->idx_arr[idx_for_arr];
        }
while ( (curr_list_node == NULL) ||
       ((curr_list_node->prev == curr_list_node) && (curr_list_node->next == curr_list_node)) );

usr_iter->list_node = curr_list_node->next;
return usr_iter;
}

iterator_t* move_prev (iterator_ptr usr_iter)
{
size_t idx_for_arr = 0;
list_node_t*  curr_list_node = NULL;
hash_elem_t*  curr_elem      = NULL;
hash_table_t* curr_table     = NULL;

if (usr_iter == NULL)
        {
        my_errno = wrong_args;
        return NULL;
        }

#if (IS_DEBUG_ON == 1)
if (usr_iter->hash_table == NULL)
        assert (!"internal error in move_next: usr_iter->hash_table == NULL");
if (usr_iter->list_node == NULL)
        assert (!"internal error in move_next: usr_iter->list_node == NULL");
#endif

curr_list_node = usr_iter->list_node;
curr_elem      = container_of (usr_iter->list_node, hash_elem_t, list_node);
curr_table     = (hash_table_t*)usr_iter->hash_table;

idx_for_arr = curr_table->func (curr_elem->key, curr_elem->key_size, curr_table->size);
if (idx_for_arr >= curr_table->size)
        {
        my_errno = wrong_hash_func;
        return NULL;
        }

if (curr_list_node->prev != curr_table->idx_arr[idx_for_arr])
        {
        usr_iter->list_node = curr_list_node->prev;
        return usr_iter;
        }

do
        {
        if (idx_for_arr >= 1)
                idx_for_arr--;
        else
                idx_for_arr = curr_table->size - 1;

        curr_list_node = curr_table->idx_arr[idx_for_arr];
        }
while ( (curr_list_node == NULL) ||
       ((curr_list_node->prev == curr_list_node) && (curr_list_node->next == curr_list_node)) );

usr_iter->list_node = curr_list_node->prev;
return usr_iter;
}

/* -1 - error;
 *  0 - no, not end;
 *  1 - yes, is end.
*/
int is_end (const iterator_ptr usr_iter)
{
int ret_val = 0;
iterator_t *iter = NULL, *tmp_iter = NULL;

if (usr_iter == NULL)
        {
        my_errno = wrong_args;
        return -1;
        }

#if (IS_DEBUG_ON == 1)
if (usr_iter->hash_table == NULL)
        assert (!"internal error in move_next: usr_iter->hash_table == NULL");
if (usr_iter->list_node == NULL)
        assert (!"internal error in move_next: usr_iter->list_node == NULL");
#endif

// cast to avoid warnings about need 'hash_table_ptr', get 'struct hash_table *'
iter = get_iterator ((hash_table_t*)(usr_iter->hash_table));
#if (IS_DEBUG_ON == 1)
if ((iter == NULL) && (my_errno == wrong_args))
        assert (!"internal error in is_end: wrong args passed to get_iterator func");
#endif
if (iter == NULL)
        return -1; // because my_errno was set by 'get_iterator'

tmp_iter = move_prev (iter);
#if (IS_DEBUG_ON == 1)
if ((tmp_iter == NULL) && (my_errno == wrong_args))
        assert (!"internal error in is_end: wrong args passed to move_prev func");
#endif
if (tmp_iter == NULL)
        {
        ret_val = delete_iterator (&iter);
        #if (IS_DEBUG_ON == 1)
        if ((ret_val == -1) && (my_errno == wrong_args))
                assert (!"internal error in is_end: wrong args passed to delete_iterator func");
        #endif

        return -1; // because my_errno was set by 'move_prev'
        }

ret_val = memcmp (usr_iter, tmp_iter, sizeof (iterator_t));
if ((ret_val < 0) || (0 < ret_val))
        ret_val = 0; // not end
else
        ret_val = 1; // is end

#if (IS_DEBUG_ON == 1)
if (delete_iterator (&iter)) // as func returns -1 only if wrong args were passed
        assert (!"internal error in is_end: wrong args passed to delete_iterator func");
#else
delete_iterator (&iter);
#endif

return ret_val;
}

hash_key_t* get_key (const iterator_ptr usr_iter)
{
hash_key_t* key_buffer = NULL;
hash_elem_t* hash_elem = NULL;

if (usr_iter == NULL)
        {
        my_errno = wrong_args;
        return NULL;
        }

#if (IS_DEBUG_ON == 1)
if (usr_iter->hash_table == NULL)
        assert (!"internal error in get_key: usr_iter->hash_table == NULL");
if (usr_iter->list_node == NULL)
        assert (!"internal error in get_key: usr_iter->list_node == NULL");
#endif

hash_elem = container_of (usr_iter->list_node, hash_elem_t, list_node);
key_buffer = calloc (1, hash_elem->key_size);
if (key_buffer == NULL)
        goto error_calloc;

if (! memcpy (key_buffer, hash_elem->key, hash_elem->key_size))
        goto error_memcpy;

return key_buffer;


error_memcpy:
        free (key_buffer);
        #if (IS_DEBUG_ON == 1)
        key_buffer = NULL;
        #endif
error_calloc:
        my_errno = mem_err;
        return NULL;
}

hash_entry_t* get_entry (const iterator_ptr usr_iter)
{
hash_key_t* entry_buffer = NULL;
hash_elem_t* hash_elem = NULL;

if (usr_iter == NULL)
        {
        my_errno = wrong_args;
        return NULL;
        }

#if (IS_DEBUG_ON == 1)
if (usr_iter->hash_table == NULL)
        assert (!"internal error in get_entry: usr_iter->hash_table == NULL");
if (usr_iter->list_node == NULL)
        assert (!"internal error in get_entry: usr_iter->list_node == NULL");
#endif

hash_elem = container_of (usr_iter->list_node, hash_elem_t, list_node);
entry_buffer = calloc (1, hash_elem->entry_size);
if (entry_buffer == NULL)
        goto error_calloc;

if (! memcpy (entry_buffer, hash_elem->entry, hash_elem->entry_size))
        goto error_memcpy;

return entry_buffer;


error_memcpy:
        free (entry_buffer);
        #if (IS_DEBUG_ON == 1)
        entry_buffer = NULL;
        #endif
error_calloc:
        my_errno = mem_err;
        return NULL;
}
//==============================================================================

