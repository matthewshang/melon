#ifndef __VM__
#define __VM__

#include <stdint.h>

#include "value.h"
#include "vector.h"

typedef struct
{
    value_r constants;
    value_r stack;
    byte_r bytecode;
    uint8_t *ip;

} vm_t;

vm_t vm_create(byte_r code, value_r constants);
void vm_destroy(vm_t *vm);
void vm_run(vm_t *vm);

void vm_dump_constants(vm_t *vm);

#endif