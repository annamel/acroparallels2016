//
// Created by voddan on 28/02/16.
//

#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "brain_fuck_machine.h"

void brain_fuck_machine(size_t mem_size, int32_t * memory, Command * script) {
    int mem = 0;  // mem index in use
    int com = 0;  // current command (script index)
    int call_back = -1;  // back_register

    int32_t reg = 0;

    while (script[com] != ret) {
        if (DEBUG) {
            printf("%3x %4d ::: %8x | ", com, com, reg);

            for (int i = 0; i < mem_size; i++)
                printf("%c%8x ", (i == mem ? '>' : ':'), memory[i]);

            printf("\n");
            usleep(1000);
        }

        switch (script[com]) {
            case ret:
                continue;
            case non:
                break;
            case lft:
                mem = (mem - 1 + (int) mem_size) % (int) mem_size;
                break;
            case rht:
                mem = (mem + 1) % (int) mem_size;
                break;
            case upp:
                reg = memory[mem];
                break;
            case dwn:
                memory[mem] = reg;
                break;
            case add:
                reg += memory[mem];
                break;
            case sub:
                reg -= memory[mem];
                break;
            case and:
                reg &= memory[mem];
                break;
            case orr:
                reg |= memory[mem];
                break;
            case xor:
                reg ^= memory[mem];
                break;
            case inc:
                reg += 1;
                break;
            case dec:
                reg -= 1;
                break;
            case zer:
                reg = 0;
                break;
            case dbl:
                reg = reg * 2;
                break;
            case shl:
//                reg = (reg << 1) | (reg >> (32 - 1));
                reg = reg << 1;
                break;
            case shr:
//                reg = (reg >> 1) | (reg << (32 - 1));
                reg = reg >> 1;
                break;
            case mov:
                mem = reg % (int) mem_size;
                break;
            case jmp:
                com = reg;
                continue;
            case jmpz:
                if (memory[mem] == 0) {
                    com = reg;
                    continue;
                } else
                    break;
            case jmpnz:
                if (memory[mem] != 0) {
                    com = reg;
                    continue;
                } else
                    break;
            case njmp:
                com += reg;
                continue;
            case njmpz:
                if (memory[mem] == 0) {
                    com += reg;
                    continue;
                } else
                    break;
            case njmpnz:
                if (memory[mem] != 0) {
                    com += reg;
                    continue;
                } else
                    break;
            case pjmp:
                com -= reg;
                continue;
            case pjmpz:
                if (memory[mem] == 0) {
                    com -= reg;
                    continue;
                } else
                    break;
            case pjmpnz:
                if (memory[mem] != 0) {
                    com -= reg;
                    continue;
                } else
                    break;
            case call:
                call_back = com;
                com = reg;
                continue;
            case back:
                assert(call_back >= 0);
                com = call_back + 1;
                call_back = -1;
                continue;
        }

        com += 1;
    }
}