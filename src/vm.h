#ifndef __VM__
#define __VM__

#include <stdint.h>

#include "value.h"
#include "vector.h"

typedef struct callframe_s
{
    uint8_t *ret;
    function_t *func;

    struct callframe_s *last;
} callframe_t;

typedef struct
{
    value_r stack;
    uint8_t *ip;
    callframe_t *callstack;

    value_r globals;
    function_t *main_func;
} vm_t;

vm_t vm_create(function_t *f);
void vm_destroy(vm_t *vm);
void vm_run(vm_t *vm);

#endif