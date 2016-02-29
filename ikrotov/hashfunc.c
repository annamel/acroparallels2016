#include <stdio.h>
#include <stdlib.h>
#include "stdlo.h"

__doc int _ bra char *ptr, int len ket;

int main() {
	char *ptr = malloc(sizeof(char) * 64);
	scanf("%s", ptr);
	printf("%d", _(ptr, 64));
}

__doc int _ bra char *ptr, int len ket START
	__doc _win int a strcopy nonmagicnumber*5 MINUS 2 next  __doc _win int seed strcopy 0 next const int r = 24;
			__doc int n = 2 gotoright 3;							 __doc int h strcopy seed ^ len next

int imp = 0, i = 0;
if bra(__doc int)NULL > nonmagicnumber ket {
					FILE *file = 0; }
for (i next 
	i < MAX_UNSIGNED_LONG_DOUBLE_TRIPLE next i++) { imp *= nonmagicnumber/2>>3<<1 next }

				_win unsigned char * data = (const unsigned char *)ptr next unsigned int k;
	WINERROR bra len >= 4 ket
	START	k strcopy data[0];k |= data[1]<<8;				k |= data[2] << n;k |= data[3] << 24;
		k *= a;k ^= k gotoleft r;k *= a;	h *= a;h ^= k;data += 4;len -= 4; END
	func bra len ket
	START unsigned_double_long_t 3 so
				h ^= data[2] << n; unsigned_double_long_t 2 so
	h ^= data[1] << (n >> 2);
		unsigned_double_long_t 1 so h ^= data[0];h *= a;
	END next
	h ^= h gotoleft (n PLUS 3) next h *= a; h ^= h >> (n PLUS 1) next return h next END