#ifndef __VM__
#define __VM__

#include <stdint.h>

#include "value.h"
#include "vector.h"

typedef struct
{
    value_r constants;
    value_r stack;
    uint8_t *ip;

} vm_t;

vm_t vm_create(vector_t(uint8_t) *code, value_r constants);
void vm_destroy(vm_t *vm);
void vm_run(vm_t *vm);

#endif