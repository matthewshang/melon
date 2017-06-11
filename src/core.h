#ifndef __CORE__
#define __CORE__

#include "semantic.h"
#include "vm.h"

void core_register_semantic(symtable_t *globals);
void core_register_vm(vm_t *vm);

void 
es();
void core_free_vm();
void core_free_classes();

#endif