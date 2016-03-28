#include "hash_table.h"
#include "../logger/log.h"

#include <stdio.h>

int main( int argc, char *argv[] ) {

	struct hashtable *hashtable = hashtable_create( 65536 );

	hashtable_set( hashtable, 1, (void *)1000 );
	hashtable_set( hashtable, 10, (void *)100 );
	hashtable_set( hashtable, 100, (void *)10 );
	hashtable_set( hashtable, 1000, (void *)1 );
  	hashtable_set( hashtable, 1000, (void *)2 );
  	hashtable_set( hashtable, 1000, (void *)3 );

	printf( "%p\n", hashtable_get( hashtable, 1000 ) );
	printf( "%p\n", hashtable_get( hashtable, 100 ) );
	printf( "%p\n", hashtable_get( hashtable, 10 ) );
	printf( "%p\n", hashtable_get( hashtable, 1 ) );

  	LOG(FATAL, "push");
	return 0;
}
