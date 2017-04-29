#include "core.h"

#include <stdio.h>

#include "ast.h"
#include "value.h"

#define PRINTLN_SLOT 0
#define PRINT_SLOT   1

static void melon_println(value_t *args, uint8_t nargs)
{
    if (nargs > 0)
    {
        value_t v = args[0];
        switch (v.type)
        {
        case VAL_BOOL: printf("%s\n", v.i == 1 ? "true" : "false"); break;
        case VAL_INT: printf("%d\n", v.i); break;
        case VAL_FLOAT: printf("%f\n", v.d); break;
        case VAL_STR: printf("%s\n", v.s); break;
        default: break;
        }
    }
    else
    {
        printf("\n");
    }
}

static void melon_print(value_t *args, uint8_t nargs)
{
    if (nargs > 0)
    {
        value_t v = args[0];
        switch (v.type)
        {
        case VAL_BOOL: printf("%s", v.i == 1 ? "true" : "false"); break;
        case VAL_INT: printf("%d", v.i); break;
        case VAL_FLOAT: printf("f", v.d); break;
        case VAL_STR: printf("%s", v.s); break;
        default: break;
        }
    }
}

// temp stuff
static function_t *core_println;
static function_t *core_print;
static closure_t *core_println_cl;
static closure_t *core_print_cl;

static bool core_codegen_inited = false;
static bool core_vm_inited = false;

void core_register_codegen(codegen_t *gen)
{
    if (core_codegen_inited) return;
    core_codegen_inited = true;

    symtable_add_local(gen->symtable, "println");
    symtable_add_local(gen->symtable, "print");
}

void core_register_vm(vm_t *vm)
{
    if (core_vm_inited) return;
    core_vm_inited = true;

    core_println = function_native_new(melon_println);
    core_println_cl = closure_new(core_println);
    vm_set_global(vm, FROM_CLOSURE(core_println_cl), PRINTLN_SLOT);

    core_print = function_native_new(melon_print);
    core_print_cl = closure_new(core_print);
    vm_set_global(vm, FROM_CLOSURE(core_print_cl), PRINT_SLOT);
}

void core_free()
{
    if (!core_vm_inited) return;

    function_free(core_println);
    function_free(core_print);
    closure_free(core_println_cl);
    closure_free(core_print_cl);
}