//
// Created by voddan on 28/02/16.
//

#ifndef MELEHOVA_TRAININGS_BRAIN_FUCK_MACHINE_H
#define MELEHOVA_TRAININGS_BRAIN_FUCK_MACHINE_H

typedef enum {
    ret, non,
    lft, rht,
    upp, dwn,
    add, sub,
    and, orr, xor,
    inc, dec,
    zer, dbl,
    shl, shr,
    mov,
    jmp, jmpz, jmpnz,
    njmp, njmpz, njmpnz,
    pjmp, pjmpz, pjmpnz,
    call, back
} Command;

/*
 * Please welcome! BFM!
 * it does what it says it does - runs my brain-fuck program!
 *
 * The machine has memory; 1 register, moving left-right; no stack
 * It can jump on condition, move by index, has + - & | ^ operates
 *
 * @param   mem_size    size of the memory in bytes
 * @param   memory      the memory
 * @param   script      the script, in special codes
 *
 * @usage   init `mem` with values, run `scrip`, take values from `mem`
 *
 * Simple, Straightforward, Effective!
 * */
void brain_fuck_machine(size_t mem_size, int32_t * memory, Command * script);

#include <stdbool.h>

#define DEBUG (false)


#endif //MELEHOVA_TRAININGS_BRAIN_FUCK_MACHINE_H
