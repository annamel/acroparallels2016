#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

#define FAULT(REASON) \
            do { printf(REASON "\n"); exit(1);} while(0)

#define BASE 10
#define BIZZ "bizz"
#define BUZZ "buzz"

uint64_t reduce(uint64_t number);

int main(int argc, char** argv) {

    int tmp = 0, i = 0, length = 0;
    uint64_t number = 0;
    char* endptr = 0;


    if(argc < 2)
        FAULT("Not enough arguments");

    for(i = 1; i < argc; i++) {

        errno = 0;
        endptr = 0;
        number = llabs(strtoll(argv[i], &endptr, BASE));
        if(errno != 0 || *endptr != '\0') {
            printf("%s ", argv[i]);
            continue;
        }

        while(number > 8)
            number = reduce(number);

        if(number == 0 || number == 3 || number == 6) {
            printf(BIZZ);
            tmp++;
        }

        if(*(endptr - 1) == '0' || *(endptr - 1) == '5') {
            printf("%s ", &BUZZ);
            tmp = 0;
            continue;
        }

        if(tmp) {
            printf(" ");
            tmp = 0;
            continue;
        }

        printf("%s ", argv[i]);
    }

    printf("\n");

    return 0;
}

#define ROUNDS 64/2

uint64_t reduce(uint64_t number) {
    int i = 0;
    int result = 0;

    for(i = 0; i < ROUNDS; i++) {
        if(!number)
            break;

        result += (number & 0x1);
        result = result - ((number & 0x2) >> 1);

        number = number >> 2;
    }

    return (uint64_t)abs(result);
}
