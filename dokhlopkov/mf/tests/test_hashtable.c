#include "../sources/hashtable/hashtable.h"
#include <stdio.h>

int main () {
	uint64_t size = 10;
	hashtable_t *tbl = hashtable_construct (size);

	//////////////// ------ ////////////////
	for (int i = 0; i < size * 2; i++) {
    	hashtable_add (tbl, i, &i);
	}
	int count = hashtable_count (tbl);
	if (count != size * 2) {
		printf ("\n\nERROR: Test on value counts was failed.\n");
		hashtable_destruct (tbl);
		return 1;
	}

	hashtable_delete (tbl, size / 2);
	count -= hashtable_count (tbl);
	if (count != 1) {
		printf ("\n\nERROR: Test on deletion and value counts was failed.\n");
		hashtable_destruct (tbl);
		return 1;
	}

	uint32_t err = 0;
	for (int i = 0; i < size; i++) {
		if (i != size / 2) err = hashtable_delete (tbl, i);
		if (err) {
			printf ("\n\nERROR: Test on deletion was failed.\n");
			hashtable_destruct (tbl);
			return 1;
		}
	}

	//////////////// ------ ////////////////
	int v0 = 3, v1 = 4, v2 = 7;

	hashtable_add (tbl, -1, &v0);
	hashtable_add (tbl, -2, &v1);
	hashtable_add (tbl, -3, &v2);

	hval_t gv0 = hashtable_get (tbl, -1);
	hval_t gv1 = hashtable_get (tbl, -2);
	hval_t gv2 = hashtable_get (tbl, -3);

	if (&v0!=gv0 || &v1!=gv1 || &v2!=gv2) {
		printf ("\n\nERROR: Wrong values in table.\n");
		hashtable_destruct(tbl);
		return 1;
	}

	/* Checking size method */
	if (hashtable_size (tbl) != size) {
		printf ("\n\nERROR: Test on size method was failed.\n");
		hashtable_destruct (tbl);
		return 1;
	}

	// /* Deleting table. */
	hashtable_destruct (tbl);

	printf ("\n\nAll tests completed successfully.\n");
	return 0;
}
