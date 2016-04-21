#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "hashfunc.h"
#include "hash_table.h"

int compare(void* item1, void* item2) {
    return 1;
}

struct Item {
    int item;
};

int get_sign(void* item) {
    return ((struct Item*)item) -> item;
}


int main()
{
    struct Item data[10000] = {0};


    struct HashTable* table = Hash_init(101, &hash_func, &get_sign);

    for(int i = 0; i < 10000; i++) {
        data[i].item = i;
        add_item(table, &data[i]);
    }

    for(int i = 0; i < 5; i++)
        add_item(table, &data[i]);

    Hash_destruct(table);

    printf("Success \n");
    return 0;
}
