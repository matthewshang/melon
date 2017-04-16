#include "core.h"

#include <stdio.h>

#include "ast.h"
#include "value.h"

#define PRINTLN_SLOT 0

static void melon_println(value_t *args, uint8_t nargs)
{
    if (nargs > 0)
    {
        value_t v = args[0];
        switch (v.type)
        {
        case VAL_BOOL: printf("%s\n", v.i == 1 ? "true" : "false"); break;
        case VAL_INT: printf("%d\n", v.i); break;
        case VAL_STR: printf("%s\n", v.s); break;
        default: break;
        }
    }
}

static function_t *core_println;

static bool core_codegen_inited = false;
static bool core_vm_inited = false;

void core_register_codegen(codegen_t *gen)
{
    if (core_codegen_inited) return;
    core_codegen_inited = true;
    symtable_add_local(gen->symtable, "println");
}

void core_register_vm(vm_t *vm)
{
    if (core_vm_inited) return;
    core_vm_inited = true;

    core_println = function_native_new(melon_println);
    vm_set_global(vm, FROM_FUNC(core_println), PRINTLN_SLOT);
}

void core_free()
{
    if (!core_vm_inited) return;
    function_free(core_println);
}