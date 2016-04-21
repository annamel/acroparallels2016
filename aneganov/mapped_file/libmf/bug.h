#ifndef __MF_BUG_H__
#define __MF_BUG_H__

#include <stdlib.h>
#include <assert.h>

#include "mfdef.h"
#include "log.h"

#define BUG_ON(cond) \
	do { \
		if(unlikely(cond)) { \
			log_write(LOG_FATAL, "BUG at %s, function %s, line %d: assertion failed\n", __FILE__, __FUNCTION__, __LINE__); \
			abort(); \
		} \
	} while(0)

#define BUILD_BUG_ON(cond) \
	static_assert(!(cond), #cond)

#endif