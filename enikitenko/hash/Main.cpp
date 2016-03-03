#include "CHashTable.h"
#include <stdint.h>
#include "HashFunction.h"

#define NUM_ROWS 64

int main()
{
	uint32_t c;
	scanf("%u", &c);
	printf("Hash: %X\n", hash(c));
}
