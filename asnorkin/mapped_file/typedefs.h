#ifndef TYPEDEFS_H
#define TYPEDEFS_H



typedef struct chunk chunk_t;
typedef struct chpool chpool_t;
typedef struct sorted_set sset_t;
typedef struct sset_item ss_item_t;
typedef struct item item_t;
typedef struct hash_table htable_t;
typedef uint32_t hkey_t;
typedef hkey_t hfunc_t(const hvalue_t,  uint32_t);
typedef chunk_t * hvalue_t;



#endif // TYPEDEFS_H
