//
// Created by kir on 26.02.16.
//

#ifndef HASHTABLE_VM_SERVICE_H
#define HASHTABLE_VM_SERVICE_H

#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

enum {
    vm_func_err_stack_has_no_enough_args = -1,
    vm_func_err_stack_overflow = 1,
    vm_func_err_bad_vm_instr = -2,
    vm_func_success = 0,
    vm_cmd_push = 0x7de8ccc4,
    vm_cmd_sum = 0xc45c53ee,
    vm_cmd_lshift = 0x156bf663,
    vm_cmd_rshift = 0xd09ff5be,
    vm_cmd_xor = 0xfe4dd717,
    vm_cmd_mem_in = 0xb667dedb,
    vm_cmd_mem_out = 0xba17dfdb
};

const uint32_t stack_max_size = 256;
typedef struct {
    uint32_t stack[stack_max_size];
    uint32_t stack_used;
    uint32_t mem;
} vm_service;

vm_service *vm_service_construct();
void vm_service_destruct(vm_service *vm);
uint32_t vm_service_mem_read(vm_service *vm);
int vm_service_push(vm_service *vm, uint32_t data);
int vm_service_sum(vm_service *vm);
int vm_service_lshift(vm_service *vm);
int vm_service_rshift(vm_service *vm);
int vm_service_xor(vm_service *vm);
int vm_service_run(vm_service *vm, const uint32_t *cmd_buff, const uint32_t cmd_buff_size);

#endif //HASHTABLE_VM_SERVICE_H
