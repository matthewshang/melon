#ifndef __VM__
#define __VM__

#include <stdint.h>

#include "value.h"
#include "vector.h"

typedef struct callframe_s
{
    uint8_t *ret;
    closure_t *closure;
    uint16_t bp;

} callframe_t;

typedef vector_t(callframe_t) callstack_t;

typedef struct
{
    value_t *stack;
    value_t *stacktop;
    size_t stacksize;

    upvalue_t *upvalues;

    uint8_t *ip;
    callstack_t callstack;

    value_r mem;
    value_r globals;
    function_t *main_func;
} vm_t;

vm_t vm_create(function_t *f);
void vm_set_global(vm_t *vm, value_t val, uint16_t idx);
void vm_destroy(vm_t *vm);
void vm_run(vm_t *vm);

#endif