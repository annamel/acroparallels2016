    #include <stdio.h>
#include <time.h>
#include <assert.h>
#include "hash_table.h"
#include "logger.h"

struct pool_item {
    void *mapped_area;
    unsigned reference_counter;
};

uint64_t hash_func(const void *cmp_identity);
int hash_cmp_func(const void *cmp_identity_a, const void *cmp_identity_b);

int main() {
    init_log_system("log.txt");

    const uint32_t hash_table_size = 128; //it's a power of two
    const uint32_t rval_fill_amount = UINT32_MAX / 5526; //it's a power of two

    hash_t *hash = hash_construct(hash_table_size, hash_func, hash_cmp_func, sizeof(pool_item));
                                                                                                                                                                                                                                                                assert(hash);
    pool_item temp;
    srand(time(NULL));
    for (int i = 0; i < rval_fill_amount; i++) {
        temp.reference_counter = rand() % 100;
        temp.mapped_area = (void *)rand();
        hash_add(hash, (const void *)(rand() % 100), (const void *)&temp);
    }
    
    hash_destruct(hash);

    deinit_log_system();
    
    return 0;
}


uint64_t hash_func(const void *cmp_identity) {
    uint64_t key = (uint64_t)cmp_identity;
    key ^= key >> 33;
    key *= 0xff51afd7ed558ccd;
    key ^= key >> 33;
    key *= 0xc4ceb9fe1a85ec53;
    key ^= key >> 33;
    return key;
}

int hash_cmp_func(const void *cmp_identity_a, const void *cmp_identity_b) {
	if (cmp_identity_a == cmp_identity_b)
        return 0;
    else
        return 1;
}