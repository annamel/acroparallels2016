#ifndef __MFDEF_H__
#define __MFDEF_H__

#include <sys/types.h>

#define likely(x)	  __builtin_expect(!!(x), 1)
#define unlikely(x)	  __builtin_expect(!!(x), 0)
#define __must_check  __attribute__((warn_unused_result))

#define min(x,y) \
	({ \
		typeof(x) _x = (x); \
		typeof(y) _y = (y); \
		_x < _y? _x : _y;   \
	})

#endif