//
// Created by kir on 26.02.16.
//

#include "vm_service.h"

vm_service *vm_service_construct() {
    return  (vm_service *)calloc(1, sizeof(vm_service));
}

void vm_service_destruct(vm_service *vm) {
    assert(vm);
    free(vm);
}

uint32_t vm_service_mem_read(vm_service *vm) {
    assert(vm);
    return vm->mem;
}

int vm_service_push(vm_service *vm, uint32_t data) {
    assert(vm);
    if (vm->stack_used >= stack_max_size)
        return vm_func_err_stack_overflow;

    vm->stack[vm->stack_used++] = data;
    return vm_func_success;
}

int vm_service_sum(vm_service *vm) {
    assert(vm);
    if (vm->stack_used < 2)
        return vm_func_err_stack_has_no_enough_args;

    vm->stack[vm->stack_used - 2] += vm->stack[vm->stack_used - 1];
    vm->stack_used--;
    return  vm_func_success;
}

int vm_service_lshift(vm_service *vm) {
    assert(vm);
    if (vm->stack_used < 2)
        return vm_func_err_stack_has_no_enough_args;

    vm->stack[vm->stack_used - 2] <<= vm->stack[vm->stack_used - 1];
    vm->stack_used--;
    return vm_func_success;
}

int vm_service_rshift(vm_service *vm) {
    assert(vm);
    if (vm->stack_used < 2)
        return vm_func_err_stack_has_no_enough_args;

    vm->stack[vm->stack_used - 2] >>= vm->stack[vm->stack_used - 1];
    vm->stack_used--;
    return vm_func_success;
}

int vm_service_xor(vm_service *vm) {
    assert(vm);
    if (vm->stack_used < 2)
        return vm_func_err_stack_has_no_enough_args;

    vm->stack[vm->stack_used - 2] ^= vm->stack[vm->stack_used - 1];
    vm->stack_used--;
    return vm_func_success;
}

int vm_service_mem_in(vm_service *vm) {
    assert(vm);
    if (vm->stack_used < 1)
        return vm_func_err_stack_has_no_enough_args;

    vm->mem = vm->stack[vm->stack_used-1];
    return vm_func_success;
}

int vm_service_mem_out(vm_service *vm) {
    assert(vm);
    return vm_service_push(vm, vm->mem);
}


int vm_service_run(vm_service *vm, const uint32_t *cmd_buff, const uint32_t cmd_buff_size) {
    assert(vm);
    uint32_t i = 0;
    int cmd_run_ret_val;

    while (i < cmd_buff_size) {
        switch (cmd_buff[i]) {
            case vm_cmd_push:
                if (i + 1 >= cmd_buff_size)
                    return vm_func_err_bad_vm_instr;
                cmd_run_ret_val = vm_service_push(vm, cmd_buff[i+1]);
                i++;
                break;
            case vm_cmd_lshift:
                cmd_run_ret_val = vm_service_lshift(vm);
                break;
            case vm_cmd_rshift:
                cmd_run_ret_val = vm_service_rshift(vm);
                break;
            case vm_cmd_sum:
                cmd_run_ret_val = vm_service_sum(vm);
                break;
            case vm_cmd_xor:
                cmd_run_ret_val = vm_service_xor(vm);
                break;
            case vm_cmd_mem_in:
                cmd_run_ret_val = vm_service_mem_in(vm);
                break;
            case vm_cmd_mem_out:
                cmd_run_ret_val = vm_service_mem_out(vm);
                break;
            default:
                return  vm_func_err_bad_vm_instr;
        }
        i++;
        if (cmd_run_ret_val != vm_func_success)
            return cmd_run_ret_val;
    }

    return vm_func_success;
}
