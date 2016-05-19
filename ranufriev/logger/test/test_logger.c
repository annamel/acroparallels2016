#include <stdio.h>
#include <malloc.h>
#include "../ring_buf/ring_buf.h"
#include "../logger/logger.h"


#include <pthread.h>
typedef struct Ring_buf
        {
        size_t size_of_buf;
        /*
        ring_buf_entry_ptr* buf;

        size_t size_of_entry;
        */
        ring_buf_elem_t* buf_of_elems;

        // this var is used to speed up 'get' func in critical section
        size_t max_size_of_entry;

        size_t head;
        pthread_mutex_t head_mtx;
        int is_head_mtx_destroyed;

        size_t tail;
        pthread_mutex_t tail_mtx;
        int is_tail_mtx_destroyed;

        // I will try w/o count_mtx
        size_t count;
        } ring_buf_t;

typedef struct Logger
        {
        int is_constructed;

        log_level_t min_log_level;
        int fd;

        ring_buf_ptr ring_buf;

        pthread_t       dumping_thread;
        pthread_cond_t  dumping_cond;
        pthread_mutex_t dumping_mtx; // not used really, only for 'dumping_cond'
        } logger_t;

#define BUF_SIZE 8

int main ()
{
int ret_val = 0;
/*
int i = 0;
char str[] = "Hello, ring_buf!";
ring_buf_ptr test = NULL;
ring_buf_N_elems_ptr test_N_elem = NULL;
ring_buf_elem_ptr test_elem = NULL;
*/

// printf ("Hello, ring_buf!\n");
// printf ("sizeof (ring_buf_t) = %zu\n", sizeof (ring_buf_t));
// printf ("sizeof (ring_buf_elem_t) = %zu\n", sizeof (ring_buf_elem_t));
// printf ("sizeof (void*) = %zu\n", sizeof (void*));

/*
ret_val = Ring_buf_construct (&test, BUF_SIZE);
printf ("Ring_buf_construct ret_val = %d\n", ret_val);
printf ("\n");

for (i = 0; i < 10; i++)
        {
        ret_val = Ring_buf_put (test, str, sizeof(str));
        // ret_val = Ring_buf_put (test, &i, sizeof(i));
        printf ("Ring_buf_put ret_val = %d\n", ret_val);
        printf ("count = %zu\n", test->count);
        printf ("head is on %zu idx.\n", test->head);
        printf ("\n");
        }
ret_val = Ring_buf_put (test, &i, sizeof(i));
printf ("Ring_buf_put ret_val = %d\n", ret_val);
printf ("count = %zu\n", test->count);
printf ("head is on %zu idx.\n", test->head);
printf ("\n");

printf ("\n");
printf ("tail is on %zu idx.\n", test->tail);
printf ("\n");

for (i = 0; i < 10; i++)
{
        ret_val = Ring_buf_get (test, &test_elem);
        printf ("Ring_buf_get ret_val = %d\n", ret_val);
        printf ("count = %zu\n", test->count);
        // printf ("str = %s\n", (char*)(test_elem->buf_for_elem));
        // printf ("numb = %d\n", *(int*)(test_elem->buf_for_entry));
        if (ret_val == 0)
                {
                printf ("str = %s or numb = %d\n", (char*)(test_elem->buf_for_entry), *(int*)(test_elem->buf_for_entry));

                // free (test_elem);
                // test_elem = NULL;
                Ring_buf_delete_elem_after_get (&test_elem);
                // free (test_elem->buf_for_entry);
                // test_elem->buf_for_entry = NULL;
                // free (test_elem);
                // test_elem = NULL;
                }
        else
                my_perror ("Ring_buf_get");
        printf ("tail is on %zu idx.\n", test->tail);
        printf ("\n");
}

for (i = 0; i < 10; i++)
        {
        printf ("%d\n", i);
        // ret_val = Ring_buf_put (test, str, sizeof(str));
        ret_val = Ring_buf_put (test, &i, sizeof(i));
        printf ("Ring_buf_put ret_val = %d\n", ret_val);
        printf ("count = %zu\n", test->count);
        printf ("head is on %zu idx.\n", test->head);
        printf ("\n");
        }
for (i = 0; i < 2; i++)
        ret_val = Ring_buf_put (test, &i, sizeof(i));
printf ("count = %zu\n", test->count);

printf ("\n");
printf ("\n");
printf ("before get_N\n");
my_errno = 0;
ret_val = Ring_buf_get_N_elems (test, &test_N_elem, 4);
printf ("Ring_buf_get_N_elems ret_val = %d\n", ret_val);
my_perror ("Ring_buf_get_N_elems");
printf ("count = %zu\n", test->count);
printf ("head is on %zu idx.\n", test->head);
printf ("tail is on %zu idx.\n", test->tail);
printf ("after get_N\n");
printf ("\n");
printf ("\n");

for (i = 0; i < test_N_elem->numb_of_elems; i++)
        {
        printf ("str = %s or numb = %d\n", (char*)(test_N_elem->buf_of_elems[i].buf_for_entry),
                                           *(int*)(test_N_elem->buf_of_elems[i].buf_for_entry));
        // test_N_elem->buf_of_elems[i].buf_for_entry;
        }

// free (test_N_elem->buf_of_elems[0].buf_for_entry);
// test_N_elem->buf_of_elems[0].buf_for_entry = NULL;
// free (test_N_elem->buf_of_elems);
// test_N_elem->buf_of_elems = NULL;
// free (test_N_elem);
// test_N_elem = NULL;

Ring_buf_delete_elems_after_get_N (&test_N_elem);

printf ("\n");
printf ("\n");

i = 42;
ret_val = Ring_buf_put (test, &i, sizeof(i));
printf ("Ring_buf_put ret_val = %d\n", ret_val);
printf ("count = %zu\n", test->count);
printf ("head is on %zu idx.\n", test->head);
printf ("\n");

ret_val = Ring_buf_get_N_elems (test, &test_N_elem, 5);
printf ("Ring_buf_get_N_elems ret_val = %d\n", ret_val);
my_perror ("Ring_buf_get_N_elems");
printf ("count = %zu\n", test->count);
printf ("head is on %zu idx.\n", test->head);
printf ("tail is on %zu idx.\n", test->tail);

for (i = 0; i < test_N_elem->numb_of_elems; i++)
        {
        printf ("str = %s or numb = %d\n", (char*)(test_N_elem->buf_of_elems[i].buf_for_entry),
                                           *(int*)(test_N_elem->buf_of_elems[i].buf_for_entry));
        // test_N_elem->buf_of_elems[i].buf_for_entry;
        }
Ring_buf_delete_elems_after_get_N (&test_N_elem);

printf ("\n");
printf ("\n");

for (i = 0; i < 10; i++)
{
        ret_val = Ring_buf_get (test, &test_elem);
        printf ("Ring_buf_get ret_val = %d\n", ret_val);
        printf ("count = %zu\n", test->count);
        // printf ("str = %s\n", (char*)(test_elem->buf_for_elem));
        // printf ("numb = %d\n", *(int*)(test_elem->buf_for_entry));
        if (ret_val == 0)
                {
                printf ("str = %s or numb = %d\n", (char*)(test_elem->buf_for_entry), *(int*)(test_elem->buf_for_entry));

                // free (test_elem);
                // test_elem = NULL;
                Ring_buf_delete_elem_after_get (&test_elem);
                // free (test_elem->buf_for_entry);
                // test_elem->buf_for_entry = NULL;
                // free (test_elem);
                // test_elem = NULL;
                }
        else
                my_perror ("Ring_buf_get");
        printf ("tail is on %zu idx.\n", test->tail);
        printf ("\n");
}

// printf ("this is the last but one str: %s\n", test->buf_of_elems[8].buf_for_elem);
printf ("this is the last str: %s\n", (char*)(test->buf_of_elems[9 % BUF_SIZE].buf_for_entry));
printf ("this is the last elem: %d, on idx = %d\n", *(int*)(test->buf_of_elems[10 % BUF_SIZE].buf_for_entry), 10 % BUF_SIZE);
// printf ("this should be NULL: %p\n", test->buf_of_elems[11].buf_for_elem);

printf ("\n");
ret_val = Ring_buf_destruct (&test);
printf ("Ring_buf_destruct ret_val = %d\n", ret_val);
ret_val = Ring_buf_destruct (&test);
printf ("Ring_buf_destruct ret_val = %d\n", ret_val);
my_perror ("second destruct");
*/

printf ("\nHere comes logger:\n");
if ((ret_val = Logger_construct (LOG_DEBUG, "./log_file.txt")) != 0)
        my_perror ("Logger_construct");
printf ("Logger_construct: ret_val = %d\n", ret_val);


int i = 0;
// for (i = 0; i < 97; i++)
for (i = 0; i < 980; i++)
        {
        printf ("\n");
        if ((ret_val = Logger_log (LOG_ERROR, "smth %d\n", i)) != 0)
                my_perror ("Logger_log");
        printf ("Logger_log: ret_val = %d\n", ret_val);
        }

printf ("\n");
if ((ret_val = Logger_log (LOG_FATAL, "smth2 %d\n", i)) != 0)
        my_perror ("Logger_log");
printf ("Logger_log: ret_val = %d\n", ret_val);




printf ("\n");
if ((ret_val = Logger_destruct ()) != 0)
        my_perror ("Logger_destruct");
// perror ("");
printf ("Logger_destruct: ret_val = %d\n", ret_val);


return 0;
}
