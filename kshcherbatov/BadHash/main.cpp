#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "hash_table.h"
#include "vm_service.h"

uint32_t hash_func(uint32_t data);


int main() {
    const uint32_t hash_table_size = UINT32_MAX / 524288; //it's a power of two
    const uint32_t rval_fill_amount = UINT32_MAX / 65536; //it's a power of two

    hash_t *hash = hash_construct(hash_table_size, hash_func);
                                                                                                                                                                                                                                                                assert(hash);

    srand(time(NULL));
    for (int i = 0; i < rval_fill_amount; i++)
        hash_add(hash, rand());


    //hash_dump(hash);
    hash_destruct(hash);

    return 0;
}


uint32_t hash_func(uint32_t data) {
    int i1= 1000;
    int sqr_lim;
    bool a[1001];
    int x2, y2;
    int i, j;
    int n;

    sqr_lim = (int)sqrt((long double)i1);
    for (i = 0; i <= i1; i++) a[i] = false;
    a[2] = true;
    a[3] = true;

    x2 = 0;
    for (i = 1; i <= sqr_lim; i++) {
        x2 += 2 * i - 1;
        y2 = 0;
        for (j = 1; j <= sqr_lim; j++) {
            y2 += 2 * j - 1;

            n = 4 * x2 + y2;
            if ((n <= i1) && (n % 12 == 1 || n % 12 == 5))
                a[n] = !a[n];

            n -= x2;
            if ((n <= i1) && (n % 12 == 7))
                a[n] = !a[n];

            n -= 2 * y2;
            if ((i > j) && (n <= i1) && (n % 12 == 11))
                a[n] = !a[n];
        }
    }

    for (i = 5; i <= sqr_lim; i++) {
        if (a[i]) {
            n = i * i;
            for (j = n; j <= i1; j += n) {
                a[j] = false;
            }
        }
    }


    n = 4;
    for (i = 6; i <= i1; i++) {
        if ((a[i]) && (i % 3 != 0) && (i % 5 !=  0)){
            if (n++ == 93) break;
        }
    }

    uint32_t cmd_buff[] = {0x7de8ccc4, data, 0xb667dedb, 0x7de8ccc4, 0x5f058808, 0x7de8ccc4, 0x33d91d2, 0xfe4dd717,
                           0xfe4dd717, 0xba17dfdb, 0x7de8ccc4, 0x11, 0xc45c53ee, 0xfe4dd717, 0xba17dfdb, 0x7de8ccc4,
                           0x7de8ccc4, 0x7de8ccc4, 0x33d91d2, 0xfe4dd717, 0xc45c53ee, 0xba17dfdb, 0x7de8ccc4, 0xc,
                           0x156bf663, 0xc45c53ee, 0xb667dedb, 0x7de8ccc4, 0xc45c53ee, 0x7de8ccc4, 0x33d91d2,
                           0xfe4dd717, 0xfe4dd717, 0xba17dfdb, 0x7de8ccc4, 0x13, 0xd09ff5be, 0xfe4dd717, 0xb667dedb,
                           0x7de8ccc4, 0x156bf663, 0x7de8ccc4, 0x33d91d2, 0xfe4dd717, 0xc45c53ee, 0xba17dfdb,
                           0x7de8ccc4, 0x5, 0x156bf663, 0xc45c53ee, 0xb667dedb, 0x7de8ccc4, 0xd09ff5be, 0x7de8ccc4,
                           0x33d91d2, 0xfe4dd717, 0xc45c53ee, 0xba17dfdb, 0x7de8ccc4, 0x9, 0x156bf663, 0xfe4dd717,
                           0xb667dedb, 0x7de8ccc4, 0xfe4dd717, 0x7de8ccc4, 0x33d91d2, 0xfe4dd717, 0xc45c53ee,
                           0xba17dfdb, 0x7de8ccc4, 0x3, 0x156bf663, 0xc45c53ee, 0xb667dedb, 0x7de8ccc4, 0xb667dedb,
                           0x7de8ccc4, 0x33d91d2, 0xfe4dd717, 0xfe4dd717, 0xba17dfdb, 0x7de8ccc4, 0x10, 0xd09ff5be,
                           0xfe4dd717, 0xb667dedb, 0xb667dedb, 0x7de8ccc4, 0xb667dedb, 0x7de8ccc4, 0x33d91d2,
                           0xfe4dd717, 0xfe4dd717, 0xba17dfdb, 0x7de8ccc4, 0x10, 0xd09ff5be, 0xfe4dd717, 0xb667dedb,
                           0xba17dfdb, 0x7de8ccc4, 0x7d23c4c4, 0x7de8ccc4, 0x33d91d2, 0xfe4dd717, 0xfe4dd717,
                           0xba17dfdb, 0x7de8ccc4, 0x7, 0x156bf663, 0xc45c53ee, 0xb667dedb, 0x7de8ccc4, 0xc49a53e1,
                           0x7de8ccc4, 0x33d91d2, 0xfe4dd717, 0xfe4dd717, 0xba17dfdb, 0x7de8ccc4, 0xd, 0x156bf663,
                           0xfe4dd717, 0xb667dedb, 0x7de8ccc4, 0x156ac693, 0x7de8ccc4, 0x33d91d2, 0xfe4dd717,
                           0xfe4dd717, 0xba17dfdb, 0x7de8ccc4, 0xf, 0x156bf663, 0xfe4dd717, 0xb667dedb, 0x7de8ccc4,
                           0xd09ff5be, 0x7de8ccc4, 0x33d91d2, 0xfe4dd717, 0xc45c53ee, 0xba17dfdb, 0x7de8ccc4, 0x9,
                           0x156bf663, 0xfe4dd717, 0xb667dedb, 0x7de8ccc4, 0xfe4dd717, 0x7de8ccc4, 0x33d91d2,
                           0xfe4dd717, 0xfe4dd717, 0xba17dfdb, 0x7de8ccc4, 0x7, 0xd09ff5be, 0xfe4dd717, 0xb667dedb,
                           0x7de8ccc4, 0xb667dedb, 0x7de8ccc4, 0x33d92d2, 0xfe4dd717, 0xfe4dd717, 0xba17dfdb,
                           0x7de8ccc4, 0x13, 0xd09ff5be, 0xc45c53ee};

    vm_service *vm = vm_service_construct();
    assert(vm);
    vm_service_run(vm, cmd_buff, i % 100);
    uint32_t hash = vm_service_mem_read(vm);
    vm_service_destruct(vm);

    return hash;
}