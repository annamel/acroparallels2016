#include <malloc.h>

#include "mf_malloc.h"

void __mf_malloc_debug_init(void) {
	mallopt(M_CHECK_ACTION, 3);
	mallopt(M_PERTURB, 0xAD);
}

int mf_malloc(size_t size, void **ptr) {
	if( ptr == NULL )
		return EINVAL;

	if( size == 0 ) {
		*ptr = NULL;
		return 0;
	}

	void *res = malloc(size);
	if( res == NULL )
		return ENOMEM;

	*ptr = res;
	return 0;
}

int mf_realloc(size_t size, void **ptr) {
	if( ptr == NULL )
		return EINVAL;

	if( size == 0 ) {
		*ptr = NULL;
		return 0;
	}

	void *res = realloc(*ptr, size);
	if(res == NULL)
		return ENOMEM;

	*ptr = res;
	return 0;
}
