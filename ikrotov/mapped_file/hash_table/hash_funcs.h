#ifndef HASH_FUNCS_H
#define HASH_FUNCS_H
#include <inttypes.h>
typedef uint32_t hkey_t;
typedef void * hvalue_t;
// default hash function values
#define HASH_CONST_1 0x01000193 //   16777619
#define HASH_CONST_2 0x811C9DC5 // 2166136261
#ifdef _MSC_VER
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

hkey_t hash_fnv1a(const hvalue_t key, uint32_t hash);
uint32_t fnv1a(unsigned char oneByte, uint32_t hash);



#endif // HASH_FUNCS_H
