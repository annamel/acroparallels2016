//*********************************************************
//********                                       **********
//********      Created by Alexander Snorkin     **********
//********              22.04.2016               **********
//********                                       **********
//*********************************************************
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



typedef hkey_t hfunc_t(const hvalue_t,  uint32_t);



//  This func generates uint32 hash key by uint32 key
//
//  - ARGUMENTS
//      key - unit32 key for generate the hash key
//      hash - const for generating
//
//  - RETURNED VALUE
//      all is good => hash key
hkey_t hash_fnv1a(const hkey_t key, uint32_t hash);

//  This func calculates the uint32 hash key by one byte
//  Assistance func for hash_fnv1a
uint32_t fnv1a(unsigned char oneByte, uint32_t hash);



#endif // HASH_FUNCS_H
