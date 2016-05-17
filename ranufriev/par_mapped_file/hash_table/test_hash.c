#include "hash_table.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>


// DONE: make hash_func return out of range value
// DONE: find_elem with different hash & make find_elem say 'not found'
        // to sum up: add some elems and try to found elem between these but not found (in one row)
// DONE: get_iterator with empty table
// NO WAY =(: (hash_table_destruct, h_t_delete_elem & _delete_h_e_..., delete_iterator pass &NULL maybe =) )
// WRONG: is_end from empty table; if table is empty - I don't have any iters - can't call is_end func

//==============================================================================
#define PROBAB_NUMB 100
extern unsigned seed;
//==============================================================================

size_t cool_hash_func (const hash_key_t* hash_key, size_t hash_key_size, size_t hash_table_size)
        {
        // add smth that makes this func return VERY big out of range number
        if ((rand() % PROBAB_NUMB) == 0)
                return (*(int*)hash_key) % hash_table_size;
        else
                return (2 * hash_table_size);
        }

int main (int argc, char* argv[])
{
size_t i = 0;
int ret_val = 0;
hash_table_ptr test = NULL;
iterator_ptr test_iter = NULL, test_iter2 = NULL, test_find_iter = NULL, new_test_iter = NULL, tmp_iter = NULL;

char*  key_buffer   = NULL;
char*  entry_buffer = NULL;

char key1[] = "1 this is key1";
char key2[] = "1 this is key2";
char key3[] = "3 this is key 3";
char key4[] = "4 oh, this is key 4";

char entry4[] = "oh, this is entry 4";


//------------------------------------------------------------------------------
// for rand ()
//------------------------------------------------------------------------------
char *endptr = NULL, *str = NULL;
long val = 0;
unsigned seed = 0;

if (argc <= 1)
        {
        seed = 1;
        srand (seed);
        }
else
        {
        str = argv[1];

        errno = 0;    // To distinguish success/failure after call
        val = strtol (str, &endptr, 10);

        // Check for various possible errors
        if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
                        || (errno != 0 && val == 0))
                {
                perror("strtol");
                exit(EXIT_FAILURE);
                }

        if (endptr == str)
                {
                fprintf(stderr, "No digits were found\n");
                exit(EXIT_FAILURE);
                }

        switch (val)
        {
        case 2:
                seed = 2;
                break;

        case 3:
                seed = 3;
                break;

        case 4:
                seed = 4;
                break;

        case 5:
                seed = 5;
                break;

        case 6:
                seed = 6;
                break;

        case 7:
                seed = 7;
                break;

        case 8:
                seed = 8;
                break;

        case 9:
                seed = 9;
                break;

        default:
                seed = 1;
                break;
        }

        srand (seed);
        }
//------------------------------------------------------------------------------


/*
// RETURN VALUE:  0 - if success
//               -1 - otherwise, with my_errno indicating the error
ret_val = Hash_table_construct (&test, 20, cool_hash_func);
printf ("%zd\n", i++);
test = (hash_table_ptr)0xffff;

// ret_val = Hash_table_add_elem (test, key1, sizeof (key1), key1, sizeof (key1));
printf ("%zd\n", i++);
ret_val = Hash_table_delete_elem (&test_iter2);
printf ("%zd\n", i++);

// find elem in hash_table
// test_find_iter = find_elem (test, key1, sizeof (key1));
printf ("%zd\n", i++);

// RETURN VALUE: iterator_ptr - if success
//               NULL         - otherwise, with my_errno indicating the error
// test_iter = get_iterator (test);
printf ("%zd\n", i++);
test_find_iter = (iterator_ptr) 0xffff;
ret_val = delete_iterator (&test_find_iter);
printf ("%zd\n", i++);
test_iter = (iterator_ptr)0xffff;
// test_iter2 = dup_iterator (test_iter);
printf ("%zd\n", i++);

// test_iter = move_next (test_iter);
printf ("%zd\n", i++);
// test_iter = move_prev (test_iter);
printf ("%zd\n", i++);
// ret_val = is_end (test_iter);
printf ("%zd\n", i++);

// RETURN VALUE:  pointer to required data - if success
//               -1                        - otherwise, with my_errno indicating the error
// !!! - funcs return pointer to new calloced buffer, that should be freed later
// key_buffer = get_key (test_iter);
printf ("%zd\n", i++);
// entry_buffer = get_entry (test_iter);
printf ("%zd\n", i++);
*/


//------------------------------------------------------------------------------
// construct
//------------------------------------------------------------------------------
printf ("\n");
// int Hash_table_construct (hash_table_ptr* hash_table, size_t hash_table_size, hash_func hashing_func);
// int Hash_table_destruct  (hash_table_ptr* hash_table);
ret_val = -1;
while (ret_val == -1)
        {
        my_errno = 0;
        ret_val = Hash_table_construct (&test, 20, cool_hash_func);
        }
printf ("construct ret_val = %d\n", ret_val);
my_perror ("after construct\n\t");
printf ("\n");
//------------------------------------------------------------------------------


// int Hash_table_add_elem    (hash_table_ptr hash_table, const hash_key_t* hash_key, size_t hash_key_size,
//                                                        const hash_entry_t* entry_to_add, size_t entry_size);
// int Hash_table_remove_elem (hash_table_ptr hash_table, const hash_key_t* hash_key, size_t hash_key_size);
//------------------------------------------------------------------------------
// add
//------------------------------------------------------------------------------
ret_val = -1;
while (ret_val == -1)
        {
        my_errno = 0;
        ret_val = Hash_table_add_elem (test, key1, sizeof (key1), key1, sizeof (key1));
        }
printf ("add1_1 ret_val = %d\n", ret_val);
my_perror ("after add1_1\n\t");
printf ("\n");

//------------------------------------------------------------------------------
// get iterator - equal to 'key1' elem
//------------------------------------------------------------------------------
test_iter = NULL;
while (test_iter == NULL)
        {
        my_errno = 0;
        test_iter = get_iterator (test);
        }
printf ("get_iterator test_iter = %p\n", test_iter);
my_perror ("after get_iterator\n\t");
printf ("\n");

//------------------------------------------------------------------------------
// find_elem - equal to 'key1' elem, and equals to previous get_iterator
//------------------------------------------------------------------------------
test_find_iter = NULL;
while (test_find_iter == NULL)
        {
        my_errno = 0;
        test_find_iter = find_elem (test, key1, sizeof (key1));
        }
printf ("find_elem_1 test_find_iter = %p\n", test_find_iter);
my_perror ("after find_elem_1\n\t");
printf ("\n");

//------------------------------------------------------------------------------
// assert to make sure that both iterators from two previous calls are equal
//------------------------------------------------------------------------------
printf ("before memcmp\n");
assert (! memcmp (test_iter, test_find_iter, sizeof (16)));
printf ("after memcmp\n\n");

//------------------------------------------------------------------------------
// delete iter from find_iterator call (remains only one from get_iterator call)
//------------------------------------------------------------------------------
my_errno = 0;
// func returns error only if wrong args are given
ret_val = delete_iterator (&test_find_iter);
printf ("delete_iterator_1 ret_val = %d\n", ret_val);
my_perror ("after delete_iterator_1\n\t");
printf ("\n");


//------------------------------------------------------------------------------
// duplicate iterator from get_iter call (very first one)
//------------------------------------------------------------------------------
test_iter2 = NULL;
while (test_iter2 == NULL)
        {
        my_errno = 0;
        test_iter2 = dup_iterator (test_iter);
        }
printf ("get_iterator_2 test_iter2 = %p\n", test_iter2);
my_perror ("after get_iterator_2\n\t");
printf ("\n");

//------------------------------------------------------------------------------
// delete this very first one iter
//------------------------------------------------------------------------------
my_errno = 0;
// func returns error only if wrong args are given
ret_val = delete_iterator (&test_iter);
printf ("delete_iterator_2 ret_val = %d\n", ret_val);
my_perror ("after delete_iterator_2\n\t");
printf ("\n");

//------------------------------------------------------------------------------
// delete first (and only one) elem that this second iterator from dup equals to
//------------------------------------------------------------------------------
test_iter = test_iter2;
ret_val = -1;
while (ret_val == -1)
        {
        my_errno = 0;
        ret_val = Hash_table_delete_elem (&test_iter2);
        }
printf ("Hash_table_delete_elem ret_val = %d\n", ret_val);
my_perror ("after Hash_table_delete_elem\n\t");
printf ("\n");
// printf ("\t\tafter delete test_iter = %p\n", test_iter);
// printf ("\t\tafter delete test_iter2 = %p\n", test_iter2);

//------------------------------------------------------------------------------
// try to find smth in empty table and it is OK if nothing is found
//------------------------------------------------------------------------------
test_find_iter = NULL;
while (! ((test_find_iter == NULL) && (my_errno == entry_not_found)))
        {
        my_errno = 0;
        test_find_iter = find_elem (test, key1, sizeof (key1));
        }
printf ("find_elem_2 test_find_iter = %p\n", test_find_iter);
if ((test_find_iter == NULL) && (my_errno == entry_not_found))
        printf ("after find_elem_2\n\t: no error has occurred\n");
else
        assert (! "smth is found in empty table\n");
        // my_perror ("after find_elem_2\n\t");
printf ("\n");

//N-----------------------------------------------------------------------------
// try to get_iter in empty table and it is OK it can't
//------------------------------------------------------------------------------
new_test_iter = NULL;
while (! ((new_test_iter == NULL) && (my_errno == table_empty)))
        {
        my_errno = 0;
        new_test_iter = get_iterator (test);
        }
printf ("get_iterator_0 new_test_iter = %p\n", new_test_iter);
if ((new_test_iter == NULL) && (my_errno == table_empty))
        printf ("after get_iterator_0\n\t: no error has occurred\n");
else
        assert (! "iterator is given in empty table\n");
        // my_perror ("after find_elem_2\n\t");
printf ("\n");

/*
my_errno = 0;
ret_val = Hash_table_remove_elem (test, key1, sizeof (key1));
printf ("remove_1 ret_val = %d\n", ret_val);
my_perror ("after remove_1");
printf ("\n");
*/

//------------------------------------------------------------------------------
// add elem with 'key1'
//------------------------------------------------------------------------------
ret_val = -1;
while (ret_val == -1)
        {
        my_errno = 0;
        ret_val = Hash_table_add_elem (test, key1, sizeof (key1), key1, sizeof (key1));
        }
printf ("add1_2 ret_val = %d\n", ret_val);
my_perror ("after add1_2\n\t");
printf ("\n");

//------------------------------------------------------------------------------
// get_iter, move it back and forth, and check (with find elem)
//      whether it is still the same elem
//              and delete both of iters
//------------------------------------------------------------------------------
test_iter = NULL;
while (test_iter == NULL)
        {
        my_errno = 0;
        test_iter = get_iterator (test);
        }
printf ("get_iterator_2 test_iter = %p\n", test_iter);
my_perror ("after get_iterator_2\n\t");
printf ("\n");

tmp_iter = NULL;
while (tmp_iter == NULL)
        {
        my_errno = 0;
        tmp_iter = move_next (test_iter);
        }
test_iter = tmp_iter;
printf ("move_next_1 test_iter = %p\n", test_iter);
my_perror ("after move_next_1\n\t");
printf ("\n");

tmp_iter = NULL;
while (tmp_iter == NULL)
        {
        my_errno = 0;
        tmp_iter = move_prev (test_iter);
        }
test_iter = tmp_iter;
printf ("move_prev_1 test_iter = %p\n", test_iter);
my_perror ("after move_prev_1\n\t");
printf ("\n");

//------------------------------------------------------------------------------
test_iter2 = NULL;
while (test_iter2 == NULL)
        {
        my_errno = 0;
        test_iter2 = find_elem (test, key1, sizeof (key1));
        }
printf ("find_elem_3 test_iter2 = %p\n", test_iter2);
my_perror ("after find_elem_3\n\t");
printf ("\n");

tmp_iter = NULL;
while (tmp_iter == NULL)
        {
        my_errno = 0;
        tmp_iter = move_next (test_iter2);
        }
test_iter2 = tmp_iter;
printf ("move_next_2 test_iter2 = %p\n", test_iter2);
my_perror ("after move_next_2\n\t");
printf ("\n");

tmp_iter = NULL;
while (tmp_iter == NULL)
        {
        my_errno = 0;
        tmp_iter = move_prev (test_iter2);
        }
test_iter2 = tmp_iter;
printf ("move_prev_2 test_iter2 = %p\n", test_iter2);
my_perror ("after move_prev_2\n\t");
printf ("\n");

printf ("before memcmp\n");
assert (! memcmp (test_iter, test_iter2, sizeof (16)));
printf ("after memcmp\n\n");
//------------------------------------------------------------------------------

my_errno = 0;
// func returns error only if wrong args are given
ret_val = delete_iterator (&test_iter);
printf ("delete_iterator_3 ret_val = %d\n", ret_val);
my_perror ("after delete_iterator_3\n\t");
printf ("\n");

my_errno = 0;
// func returns error only if wrong args are given
ret_val = delete_iterator (&test_iter2);
printf ("delete_iterator_4 ret_val = %d\n", ret_val);
my_perror ("after delete_iterator_4\n\t");
printf ("\n");
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// first elem still in the table, so add two more (they will lay after first one
//      due to "cool hash func")...
//------------------------------------------------------------------------------
ret_val = -1;
while (ret_val == -1)
        {
        my_errno = 0;
        ret_val = Hash_table_add_elem (test, key2, sizeof (key2), key2, sizeof (key2));
        }
printf ("add2 ret_val = %d\n", ret_val);
my_perror ("after add2\n\t");
printf ("\n");

//N-----------------------------------------------------------------------------
// try to find_elem that isn't in the table and it's OK if we can't found anything
//------------------------------------------------------------------------------
new_test_iter = NULL;
while (! ((new_test_iter == NULL) && (my_errno == entry_not_found)))
        {
        my_errno = 0;
        new_test_iter = find_elem (test, key2, sizeof (key2) - 1);
        }
printf ("find_elem_0 new_test_iter = %p\n", new_test_iter);
if ((new_test_iter == NULL) && (my_errno == entry_not_found))
        printf ("after find_elem_0\n\t: no error has occurred\n");
else
        assert (! "iterator on elem that is not in the table is given");
        // my_perror ("after find_elem_2\n\t");
printf ("\n");
//------------------------------------------------------------------------------

ret_val = -1;
while (ret_val == -1)
        {
        my_errno = 0;
        ret_val = Hash_table_add_elem (test, key3, sizeof (key3), key3, sizeof (key3));
        }
printf ("add3 ret_val = %d\n", ret_val);
my_perror ("after add3\n\t");
printf ("\n");

//------------------------------------------------------------------------------
// ... then get an iterator to the first one and move it two times forth
//              and then check if this (third) elem is end
//------------------------------------------------------------------------------
test_iter = NULL;
while (test_iter == NULL)
        {
        my_errno = 0;
        test_iter = get_iterator (test);
        }
printf ("get_iterator_3 test_iter = %p\n", test_iter);
my_perror ("after get_iterator_3\n\t");
printf ("\n");

for (i = 0; i < 2; i++)
        {
        tmp_iter = NULL;
        while (tmp_iter == NULL)
                {
                my_errno = 0;
                tmp_iter = move_next (test_iter);
                }
        }
test_iter = tmp_iter;
printf ("move_next_4 test_iter = %p\n", test_iter);
my_perror ("after move_next_4\n\t");
printf ("\n");

ret_val = -1;
while (ret_val == -1)
        {
        my_errno = 0;
        ret_val = is_end (test_iter);
        }
printf ("is_end_1 ret_val = %d\n", ret_val);
my_perror ("after is_end_1\n\t");
printf ("\n");

// printf ("%s\n",(char*)((container_of (test_iter->list_node, hash_elem_t, list_node))->entry));

assert (ret_val == 1);

//------------------------------------------------------------------------------
// I have one iter remain from last operations, so just testing
//      my get_key/entry funcs and as iter points to third elem...
//------------------------------------------------------------------------------
key_buffer = NULL;
while (key_buffer == NULL)
        {
        my_errno = 0;
        key_buffer = get_key (test_iter);
        }
printf ("get_key_1 key_buffer = %p; %s\n", key_buffer, key_buffer);
my_perror ("after get_key_1\n\t");
printf ("\n");

entry_buffer = NULL;
while (entry_buffer == NULL)
        {
        my_errno = 0;
        entry_buffer = get_entry (test_iter);
        }
printf ("get_entry_1 entry_buffer = %p; %s\n", entry_buffer, entry_buffer);
my_perror ("after get_entry_1\n\t");
printf ("\n");

//------------------------------------------------------------------------------
// ...test if this funcs return correct values
//------------------------------------------------------------------------------
assert (! (memcmp (key_buffer, entry_buffer, sizeof (key3))));

//------------------------------------------------------------------------------
// Do not forget that get_key/entry funcs return buffer that needs to be freed
//------------------------------------------------------------------------------
free (key_buffer  ); key_buffer   = NULL;
free (entry_buffer); entry_buffer = NULL;

//------------------------------------------------------------------------------
// Add last 4-th elem...
//------------------------------------------------------------------------------
ret_val = -1;
while (ret_val == -1)
        {
        my_errno = 0;
        ret_val = Hash_table_add_elem (test, key4, sizeof (key4), entry4, sizeof (entry4));
        }
printf ("add4 ret_val = %d\n", ret_val);
my_perror ("after add4\n\t");
printf ("\n");


//------------------------------------------------------------------------------
// ... and test if iter that points to third elem is end?
//              and it is OK that it is false
//------------------------------------------------------------------------------
ret_val = -1;
while (ret_val == -1)
        {
        my_errno = 0;
        ret_val = is_end (test_iter);
        }
printf ("is_end_2 ret_val = %d\n", ret_val);
my_perror ("after is_end_2\n\t");
printf ("\n");

assert (ret_val == 0);

//------------------------------------------------------------------------------
// just delete my last iter
//------------------------------------------------------------------------------
my_errno = 0;
// func returns error only if wrong args are given
ret_val = delete_iterator (&test_iter);
printf ("delete_iterator_7 ret_val = %d\n", ret_val);
my_perror ("after delete_iterator_7\n\t");
printf ("\n");


/*
my_errno = 0;
ret_val = Hash_table_remove_elem (test, key4, sizeof (key4));
printf ("remove_4 ret_val = %d\n", ret_val);
my_perror ("after remove_4");
printf ("\n");
*/

//------------------------------------------------------------------------------
// Add 4-th elem one more time and find it to get iter
//      (so now there are 5 elems, two last ones are with key&entry 4)
//------------------------------------------------------------------------------
ret_val = -1;
while (ret_val == -1)
        {
        my_errno = 0;
        ret_val = Hash_table_add_elem (test, key4, sizeof (key4), entry4, sizeof (entry4));
        }
printf ("add4 ret_val = %d\n", ret_val);
my_perror ("after add4\n\t");
printf ("\n");

test_iter2 = NULL;
while (test_iter2 == NULL)
        {
        my_errno = 0;
        test_iter2 = find_elem (test, key4, sizeof (key4));
        }
printf ("find_elem_4 test_iter2 = %p\n", test_iter2);
my_perror ("after find_elem_4\n\t");
printf ("\n");

//------------------------------------------------------------------------------
// due to that fact that I add new elems in the tail (end) of list,
// last added elem will be the end, and if I move its iter 4 times forth it
// it will points to 4-th elem, that is exactly the same as 5-th (see above)
//------------------------------------------------------------------------------
for (i = 0; i < 4; i++)
        {
        tmp_iter = NULL;
        while (tmp_iter == NULL)
                {
                my_errno = 0;
                tmp_iter = move_next (test_iter2);
                }
        }
test_iter2 = tmp_iter;
printf ("move_next_3 test_iter2 = %p\n", test_iter2);
my_perror ("after move_next_3\n\t");
printf ("\n");

//------------------------------------------------------------------------------
// I found 3-th elem and moved it 3 times backward it will points to 5th elem
// that equals to 5th...
//------------------------------------------------------------------------------
while (test_iter == NULL)
        {
        my_errno = 0;
        test_iter = find_elem (test, key3, sizeof (key3));
        }
printf ("find_elem_5 test_iter = %p\n", test_iter);
my_perror ("after find_elem_5\n\t");
printf ("\n");

for (i = 0; i < 3; i++)
        {
        tmp_iter = NULL;
        while (tmp_iter == NULL)
                {
                my_errno = 0;
                tmp_iter = move_prev (test_iter);
                }
        }
test_iter = tmp_iter;
printf ("move_prev_3 test_iter = %p\n", test_iter);
my_perror ("after move_prev_3\n\t");
printf ("\n");

//------------------------------------------------------------------------------
// ... we check it here
//------------------------------------------------------------------------------
printf ("before memcmp\n");
assert (! memcmp (test_iter, test_iter2, sizeof (16)));
printf ("after memcmp\n\n");

//------------------------------------------------------------------------------
// delete everything
//------------------------------------------------------------------------------
my_errno = 0;
// func returns error only if wrong args are given
ret_val = delete_iterator (&test_iter);
printf ("delete_iterator_5 ret_val = %d\n", ret_val);
my_perror ("after delete_iterator_5\n\t");
printf ("\n");

my_errno = 0;
// func returns error only if wrong args are given
ret_val = delete_iterator (&test_iter2);
printf ("delete_iterator_6 ret_val = %d\n", ret_val);
my_perror ("after delete_iterator_6\n\t");
printf ("\n");


my_errno = 0;
// func returns error only if wrong args are given
ret_val = Hash_table_destruct (&test);
printf ("destruct ret_val = %d\n", ret_val);
my_perror ("after destruct\n\t");
printf ("\n");
printf ("\n");
//------------------------------------------------------------------------------

//-----
/* make func not-static to test
my_errno = 0;
hash_elem = _create_hash_elem (key, sizeof (key), key, sizeof (key));
my_perror ("after _create_hash_elem ");
printf ("\n");

my_errno = 0;
ret_val = _delete_hash_elem_from_mem (&hash_elem);
printf ("_delete_hash_elem_from_mem ret_val = %d\n", ret_val);
my_perror ("after _delete_hash_elem_from_mem");
printf ("\n");
printf ("\n");
*/

/*
my_errno = 1;
my_perror ("test");
*/
// printf ("%d", errno_error_definit[4]);


//------------------------------------------------------------------------------
// try to pass 0 and NULL everywhere
//------------------------------------------------------------------------------
// RETURN VALUE:  0 - if success
//               -1 - otherwise, with my_errno indicating the error
Hash_table_construct (NULL, 0, NULL);
Hash_table_destruct  (NULL);

Hash_table_add_elem    (NULL, NULL, 0, NULL, 0);
Hash_table_delete_elem (NULL);

// find elem in hash_table
find_elem (NULL, NULL, 0);

// RETURN VALUE: iterator_ptr - if success
//               NULL         - otherwise, with my_errno indicating the error
get_iterator    (NULL);
delete_iterator (NULL);
dup_iterator    (NULL);

move_next (NULL);
move_prev (NULL);
is_end    (NULL);         // -1 - error; 0 - no, not end; 1 - yes, is end

// RETURN VALUE:  pointer to required data - if success
//               -1                        - otherwise, with my_errno indicating the error
// !!! - funcs return pointer to new calloced buffer, that should be freed later
get_key   (NULL);
get_entry (NULL);


return 0;
}

