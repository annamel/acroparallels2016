#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include "hellhash.h"

#define handle_error(msg) \
    do {perror(msg); exit(EXIT_FAILURE);} while(0)

int main(int argc, char * argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s number\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const int numeral_system_base = 10;

    char *endptr = NULL;

    errno = 0;
    long int val = strtol(argv[1], &endptr, numeral_system_base);

    if (errno != 0)
        handle_error("strtol"); /* "Value is out of range", actually */

    if (endptr == argv[1]) {
        fprintf(stderr, "No digits found\n");
        exit(EXIT_FAILURE);
    }
    else if (*endptr != '\0') {
        fprintf(stderr, "Futher characters after number\n");
        exit(EXIT_FAILURE);
    }
   
    if(val != (int32_t)val ) {
        fprintf(stderr, "Value is out of range\n");
        exit(EXIT_FAILURE);
    }
    printf("%u\n", hellhash( (int32_t)val ));
    return 0;
}
