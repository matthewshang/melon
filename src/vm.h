#ifndef __VM__
#define __VM__

#include <stdint.h>

#include "value.h"
#include "vector.h"

typedef struct callframe_s
{
    uint8_t *ret;
    closure_t *closure;
    uint32_t bp;
    bool caller_stack;

} callframe_t;

typedef vector_t(callframe_t) callstack_t;

typedef struct vm_s
{
    value_t *stack;
    value_t *stacktop;
    size_t stacksize;

    upvalue_t *upvalues;

    uint8_t *ip;
    uint32_t bp;
    callstack_t callstack;

    value_r mem;
    value_r globals;
    closure_t *closure;
} vm_t;

vm_t vm_create();
void vm_set_global(vm_t *vm, value_t val, uint16_t idx);
void vm_set_stack(vm_t *vm, value_t val, uint32_t idx);
void vm_destroy(vm_t *vm);
void vm_run_main(vm_t *vm, function_t *main);
void vm_run_closure(vm_t *vm, closure_t *cl, value_t args[], uint16_t nargs, value_t **ret);

void vm_push_mem(vm_t *vm, value_t v);

#endif