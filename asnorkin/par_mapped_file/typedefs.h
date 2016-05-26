#ifndef TYPEDEFS_H
#define TYPEDEFS_H
#include <inttypes.h>



#define SKIPLIST_ON 1



typedef struct chunk chunk_t;
typedef struct chpool chpool_t;
typedef struct item item_t;

typedef struct hash_table htable_t;
typedef uint32_t hkey_t;
typedef hkey_t hfunc_t(const hvalue_t,  uint32_t);
typedef chunk_t * hvalue_t;

typedef struct skiplist skiplist_t;
typedef struct snode snode_t;
typedef uint32_t slkey_t;
typedef chunk_t * slvalue_t;


#endif // TYPEDEFS_H
