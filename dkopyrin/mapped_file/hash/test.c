#include "hash_table.h"
#include "../logger/log.h"

#include <stdio.h>

int main( int argc, char *argv[] ) {
	struct hashtable hashtable;
	hashtable_init( &hashtable, 65536 );

	printf("%p\n", hashtable_get(&hashtable, 1000));
	printf("%p\n", hashtable_get(&hashtable, 500));

  	int i;
  	for (i = 0; i <= 11; i++){
		hashtable_set(&hashtable, i, (void *)(i*i));
	}

	printf("%p\n", hashtable_get(&hashtable, 100));
	printf("%p\n", hashtable_get(&hashtable, 10));
	printf("%p\n", hashtable_get(&hashtable, 1));

  	hashtable_finalize(&hashtable);

  	LOG(FATAL, "push");
	return 0;
}
