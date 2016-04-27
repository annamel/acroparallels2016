#include "hash_funcs.h"


hkey_t hash_fnv1a(const hvalue_t key, uint32_t hash)
{
    const unsigned char* ptr = (const unsigned char*) &key;
    hash = fnv1a(*ptr++, hash);
    hash = fnv1a(*ptr++, hash);
    hash = fnv1a(*ptr++, hash);
    return fnv1a(*ptr  , hash);
}



uint32_t fnv1a(unsigned char oneByte, uint32_t hash)
{
    return (oneByte ^ hash) * HASH_CONST_1;
}
