#ifndef __CORE__
#define __CORE__

#include "codegen.h"
#include "vm.h"

void core_register_codegen(codegen_t *gen);
void core_register_vm(vm_t *vm);
void core_free();

#endif