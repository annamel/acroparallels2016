//
// Created by voddan on 27/02/16.
//

#include <stdio.h>
#include <mach_debug/zone_info.h>

#include "brain_fuck_machine.h"

/*
 * Ok, so it turned out I am completely unable to produce ugly code
 * Moreover, I see no point in it, since anybody can use an auto-formatter on it
 * So instead here is a piece of perfectly fine code, which gonna hurt your brain REALLY, REALLY BAD
 * If you think vaseline will help, it's the time to put it on. BUAHAHA
 * */

/* do you read brain-fuck? No? You should! It's fun ;) */

// {1, 2, 3, 4, 5, 0, 0}
Command program_shift_5[] = {
/*0*/       lft, lft, lft,
/*3*/       upp, rht, dwn, lft, lft,
/*8*/       zer, inc, dbl, inc, jmpnz,
/*13*/      lft, upp, rht, rht, dwn, lft, lft, zer, dwn,

/**/        non, ret
};


// {0, 0, key, 0, 0, 0, 0}
Command program_hash[] = {
/*0*/       zer, inc, dbl, dbl, inc, dbl, dbl, njmp,
/*8*/       rht, rht, upp, shl, dwn, zer, mov, upp, dec, dwn,
/*18*/      zer, inc, dbl, dbl, dbl, dbl, inc, pjmpnz, back,

/*27*/      zer, inc, dbl, dbl, inc, dbl, dbl, njmp,
/*35*/      rht, rht, upp, shr, dwn, zer, mov, upp, dec, dwn,
/*45*/      zer, inc, dbl, dbl, dbl, dbl, inc, pjmpnz, back,

            zer, inc, dbl, dbl, dbl, lft, lft, dwn,
            dbl, inc, dbl, inc, rht, dwn, rht,

            zer, inc, dbl, dbl, dbl, dbl, dec, dwn,
            rht, rht, upp, rht, dwn, zer, mov,
            inc, dbl, dbl, dbl, call,
            rht, rht, upp, rht, sub, dec,
            dwn, lft, dwn, lft, lft,

            zer, inc, dbl, inc, dbl, dbl, dwn,
            lft, upp, rht, call,
            rht, rht, upp, rht, xor,
            dwn, lft, dwn, lft, lft,

            rht, rht, upp, shl, shl, add, dwn, rht, dwn,
            shr, shr, shr, shr, xor, dwn, lft, dwn,

            shl, shl, shl, add, rht, dwn, lft, lft, lft,
            zer, inc, dbl, dbl, inc, dbl, inc, dwn,
            lft, lft, upp, rht, rht, call,
            rht, rht, upp, rht, add, dwn,
            lft, dwn, zer, mov,

            inc, dbl, dbl, dbl, dbl, dwn,
            inc, dbl, inc, call,
            zer, inc, dbl, mov, upp, rht, xor, dwn,
            zer, mov,

/**/        non, ret
};


int32_t hash_function_vodopian_bfm(int32_t key) {
    int32_t mem[] = {0, 0, key, 0, 0, 0, 0};
    int size = sizeof(mem) / sizeof(mem[0]);

    brain_fuck_machine(size, mem, program_hash);


    return mem[3];
}




