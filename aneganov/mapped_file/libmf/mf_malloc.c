#include <stdlib.h>
#include <string.h>

#include "mf_malloc.h"
#include "log.h"

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

int mf_zmalloc(size_t size, void **ptr) {
	if( ptr == NULL )
		return EINVAL;

	if( size == 0 ) {
		*ptr = NULL;
		return 0;
	}

	void *res = malloc(size);
	if(res == NULL)
		return ENOMEM;

	memset(res, 0, size);

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

int mf_free(size_t size, void **ptr) {
	if(ptr == NULL)
		return EINVAL;

#ifdef DEBUG2
	if(*ptr != NULL)
		memset(*ptr, MF_MALLOC_POISON, size);
#endif

	free(*ptr);
	*ptr = NULL;

	return 0;
}
