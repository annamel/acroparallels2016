#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint32_t hash_func(int input)
{
    //middle-square

#define MAGIC_NUMBER 3779

    uint32_t result = ((uint32_t)input) * MAGIC_NUMBER;

    for(int i = 0; i < 10; i++) {
        if(result < 10000)
            result *= MAGIC_NUMBER;

        result = result * result;
        result = (result % 10000000) / 10000;
    }

    return result * result;
}
