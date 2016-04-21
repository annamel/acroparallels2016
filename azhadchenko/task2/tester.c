#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#define BUFF_LEN 271

int main(){

    int a = 0;
    size_t pagesize = sysconf(_SC_PAGESIZE);
    uint32_t (*hash_func)(int) = 0;


    char* buff = (char*)calloc(1, pagesize);

    a = open("hash_binary", O_RDWR);
    read(a, buff, BUFF_LEN);

    hash_func = buff;
    buff = (char*)(((size_t)buff) & ~(pagesize - 1));

    mprotect(buff, pagesize, PROT_READ|PROT_WRITE|PROT_EXEC);

    scanf("%d", &a);
    printf("%u \n", hash_func(a));

    return 0;
}
