#include <stdlib.h>
void *memtest_alloc(size_t size, int sucprob) {
    if(sucprob > 100 || sucprob < 0) return NULL;
    int coin = rand() % 100;
    if(coin < sucprob) return malloc(size);
    else return NULL;
}
void *memtest_realloc(void *ptr, size_t size, int sucprob) {
    if(sucprob > 100 || sucprob < 0) return NULL;
    int coin = rand() % 100;
    if(coin < sucprob) return realloc(ptr, size);
    else return NULL;
}
