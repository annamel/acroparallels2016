#include <stdint.h>
#include "HashFunction.h"
#include <stdio.h>

#define NUM_ROWS 64

int main()
{
	uint32_t c = 1;
	//scanf("%u", &c);
	printf("Hash: %X\n", hash(c));
}
