#ifndef __CORE__
#define __CORE__

#include "semantic.h"
#include "vm.h"

#define CORE_LOADF_STRING "$loadfield"
#define CORE_LOADAT_STRING "$loadat"
#define CORE_STOREF_STRING "$storefield"
#define CORE_STOREAT_STRING "$storeat"
#define CORE_NEW_STRING "$new"
#define CORE_INIT_STRING "$init"

void core_register_semantic(symtable_t *globals);
void core_register_vm(vm_t *vm);

void core_init_classes();
void core_free_vm();
void core_free_classes();

#endif