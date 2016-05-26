#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include "logger/logger.h"
#include "chunk_manager/chunk_manager.h"
#include "mapped_file/mapped_file.h"
#include "hash_table/hash_table.h"


/*int main(void)
{
    int error = 0;
    printf("\n\tStart testing..\n\n");



    int fd = open("test.txt", O_RDWR, 0666);
    if(fd == -1)
        printf("can't open file\n");

    chpool_t *chp = chp_init(fd, PROT_READ | PROT_WRITE);
    if(!chp)
        printf("can't init chpool\n");

    chunk_t *ch1 = ch_init(1, 1, chp);
    if(!ch1)
        printf("can't init chunk\n");
ht_print_table(chp->ht);
    chunk_t *ch2 = ch_init(1, 2, chp);
    if(!ch2)
        printf("can't init chunk\n");
ht_print_table(chp->ht);
    chunk_t *ch3 = ch_init(1, 3, chp);
    if(!ch3)
        printf("can't init chunk\n");
ht_print_table(chp->ht);
    chunk_t *ch4 = ch_init(1, 4, chp);
    if(!ch4)
        printf("can't init chunk\n");
ht_print_table(chp->ht);
    chunk_t *ch6 = ch_init(1, 5, chp);
    if(!ch6)
        printf("can't init chunk\n");
ht_print_table(chp->ht);
    chunk_t *ch7 = ch_init(1, 6, chp);
    if(!ch7)
        printf("can't init chunk\n");
ht_print_table(chp->ht);
    chunk_t *ch8 = ch_init(10, 4, chp);
    if(!ch8)
        printf("can't init chunk\n");
ht_print_table(chp->ht);
    chunk_t *ch9 = ch_init(10, 5, chp);
    if(!ch9)
        printf("can't init chunk\n");
ht_print_table(chp->ht);
    chunk_t *ch10 = ch_init(3, 3, chp);
    if(!ch10)
        printf("can't init chunk\n");
ht_print_table(chp->ht);
    chunk_t *ch11 = ch_init(0, 1, chp);
    if(!ch11)
        printf("can't init chunk\n");
ht_print_table(chp->ht);
    chunk_t *ch12 = ch_init(0, 2, chp);
    if(!ch12)
        printf("can't init chunk\n");
ht_print_table(chp->ht);

chunk_t *ch14 = ch_init(0, 3, chp);
if(!ch14)
    printf("can't init chunk\n");

chunk_t *ch15 = ch_init(10, 7, chp);
if(!ch15)
    printf("can't init chunk\n");

chunk_t *ch16 = ch_init(10, 6, chp);
if(!ch16)
    printf("can't init chunk\n");

chunk_t *ch17 = ch_init(0, 5, chp);
if(!ch17)
    printf("can't init chunk\n");

chunk_t *ch18 = ch_init(0, 4, chp);
if(!ch18)
    printf("can't init chunk\n");

chunk_t *ch19 = ch_init(3, 1, chp);
if(!ch19)
    printf("can't init chunk\n");

chunk_t *ch20 = ch_init(3, 2, chp);
if(!ch20)
    printf("can't init chunk\n");

chunk_t *ch22 = ch_init(4, 4, chp);
if(!ch22)
    printf("can't init chunk\n");

chunk_t *ch23 = ch_init(4, 1, chp);
if(!ch23)
    printf("can't init chunk\n");

chunk_t *ch24 = ch_init(4, 3, chp);
if(!ch24)
    printf("can't init chunk\n");

chunk_t *ch25 = ch_init(4, 2, chp);
if(!ch25)
    printf("can't init chunk\n");
// One more time
chunk_t *ch26 = ch_init(1, 1, chp);
if(!ch26)
    printf("can't init chunk\n");

chunk_t *ch27 = ch_init(1, 2, chp);
if(!ch27)
    printf("can't init chunk\n");

chunk_t *ch28 = ch_init(1, 3, chp);
if(!ch28)
    printf("can't init chunk\n");

chunk_t *ch29 = ch_init(1, 4, chp);
if(!ch29)
    printf("can't init chunk\n");

chunk_t *ch30 = ch_init(1, 5, chp);
if(!ch30)
    printf("can't init chunk\n");

chunk_t *ch31 = ch_init(1, 6, chp);
if(!ch31)
    printf("can't init chunk\n");

chunk_t *ch32 = ch_init(10, 4, chp);
if(!ch32)
    printf("can't init chunk\n");

chunk_t *ch33 = ch_init(10, 5, chp);
if(!ch33)
    printf("can't init chunk\n");

chunk_t *ch34 = ch_init(3, 3, chp);
if(!ch34)
    printf("can't init chunk\n");

chunk_t *ch35 = ch_init(0, 1, chp);
if(!ch35)
    printf("can't init chunk\n");

chunk_t *ch36 = ch_init(0, 2, chp);
if(!ch36)
    printf("can't init chunk\n");

chunk_t *ch37 = ch_init(0, 3, chp);
if(!ch37)
    printf("can't init chunk\n");

chunk_t *ch38 = ch_init(10, 7, chp);
if(!ch38)
    printf("can't init chunk\n");

chunk_t *ch39 = ch_init(10, 6, chp);
if(!ch39)
    printf("can't init chunk\n");

chunk_t *ch40 = ch_init(0, 5, chp);
if(!ch40)
    printf("can't init chunk\n");

chunk_t *ch41 = ch_init(0, 4, chp);
if(!ch41)
    printf("can't init chunk\n");

chunk_t *ch42 = ch_init(3, 1, chp);
if(!ch42)
    printf("can't init chunk\n");

chunk_t *ch43 = ch_init(3, 2, chp);
if(!ch43)
    printf("can't init chunk\n");

chunk_t *ch44 = ch_init(4, 4, chp);
if(!ch44)
    printf("can't init chunk\n");

chunk_t *ch45 = ch_init(4, 1, chp);
if(!ch45)
    printf("can't init chunk\n");

chunk_t *ch46 = ch_init(4, 3, chp);
if(!ch46)
    printf("can't init chunk\n");

chunk_t *ch47 = ch_init(4, 2, chp);
if(!ch47)
    printf("can't init chunk\n");
ht_print_table(chp->ht);

chunk_t *fitem;
error = ht_find(chp->ht, 0, 3, &fitem);
if(error)
    printf("can't find item\n");
else
    printf("finded item: %u\n", fitem);

error = ht_find(chp->ht, 1, 4, &fitem);
if(error)
    printf("can't find item\n");
else
    printf("finded item: %u\n", fitem);

error = ht_find(chp->ht, 3, 2, &fitem);
if(error)
    printf("can't find item\n");
else
    printf("finded item: %u\n", fitem);

error = ht_find(chp->ht, 10, 4, &fitem);
if(error)
    printf("can't find item\n");
else
    printf("finded item: %u\n", fitem);

error = ht_find(chp->ht, 10, 7, &fitem);
if(error)
    printf("can't find item\n");
else
    printf("finded item: %u\n", fitem);

error = ht_find(chp->ht, 4, 4, &fitem);
if(error)
    printf("can't find item\n");
else
    printf("finded item: %u\n", fitem);

error = ht_find(chp->ht, 4, 1, &fitem);
if(error)
    printf("can't find item\n");
else
    printf("finded item: %u\n", fitem);

error = ht_find(chp->ht, 1, 6, &fitem);
if(error)
    printf("can't find item\n");
else
    printf("finded item: %u\n", fitem);

error = ht_find(chp->ht, 3, 2, &fitem);
if(error)
    printf("can't find item\n");
else
    printf("finded item: %u\n", fitem);


    error = ht_del(chp->ht, 0, 3);
    if(error)
        printf("can't del item\n");
    ht_print_table(chp->ht);

    error = ht_del(chp->ht, 1, 6);
    if(error)
        printf("can't del item\n");
    ht_print_table(chp->ht);

    error = ht_del(chp->ht, 10, 4);
    if(error)
        printf("can't del item\n");
    ht_print_table(chp->ht);

    error = ht_del(chp->ht, 3, 3);
    if(error)
        printf("can't del item\n");
    ht_print_table(chp->ht);

    error = ht_del(chp->ht, 3, 3);
    if(error)
        printf("can't del item\n");
    ht_print_table(chp->ht);

    error = ht_del(chp->ht, 1, 5);
    if(error)
        printf("can't del item\n");
    ht_print_table(chp->ht);

    error = ht_del(chp->ht, 1, 1);
    if(error)
        printf("can't del item\n");
    ht_print_table(chp->ht);

    error = ht_del(chp->ht, 1, 2);
    if(error)
        printf("can't del item\n");
    ht_print_table(chp->ht);

    error = ht_del(chp->ht, 1, 4);
    if(error)
        printf("can't del item\n");
    ht_print_table(chp->ht);

    error = ht_del(chp->ht, 1, 3);
    if(error)
        printf("can't del item\n");
    ht_print_table(chp->ht);

    error = ht_del(chp->ht, 4, 4);
    if(error)
        printf("can't del item\n");
    ht_print_table(chp->ht);




    error = chp_deinit(chp);
    if(error)
        printf("can't deinit chpool, error=%d\n", error);


    printf("\n\tFinish testing!\n");
    return 0;
}*/
