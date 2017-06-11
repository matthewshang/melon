#include "core.h"

#include <stdio.h>

#include "ast.h"
#include "value.h"

#define PRINTLN_SLOT 0
#define PRINT_SLOT   1
#define RETURN_VALUE(val)                                                               \
    do {                                                                                \
        vm_set_stack(vm, val, retidx);                                                  \
        return;                                                                         \
    } while (0)

static void melon_println(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    if (nargs > 0)
    {
        value_t v = args[0];
        switch (v.type)
        {
        case VAL_BOOL: printf("%s\n", AS_BOOL(v) == 1 ? "true" : "false"); break;
        case VAL_INT: printf("%d\n", AS_INT(v)); break;
        case VAL_STR: printf("%s\n", AS_STR(v)); break;
        case VAL_FLOAT: printf("%f\n", AS_FLOAT(v)); break;
        default: break;
        }
    }
    else
    {
        printf("\n");
    }
}

static void melon_print(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    if (nargs > 0)
    {
        value_t v = args[0];
        switch (v.type)
        {
        case VAL_BOOL: printf("%s", AS_BOOL(v) == 1 ? "true" : "false"); break;
        case VAL_INT: printf("%d", AS_INT(v)); break;
        case VAL_STR: printf("%s", AS_STR(v)); break;
        case VAL_FLOAT: printf("%f", AS_FLOAT(v)); break;
        default: break;
        }
    }
}

static void object_classname(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    value_t v = args[0];
    if (IS_INSTANCE(v))
    {
        value_t name = FROM_CSTR(AS_INSTANCE(v)->c->identifier);
        RETURN_VALUE(name);
    }
}

// temp stuff
static function_t *core_println;
static function_t *core_print;
static closure_t *core_println_cl;
static closure_t *core_print_cl;

static bool core_classes_inited = false;
static bool core_vm_inited = false;
static bool core_semantic_inited = false;

void core_register_semantic(symtable_t *globals)
{
    if (core_semantic_inited) return;
    core_semantic_inited = true;

    symtable_add_local(globals, "println");
    symtable_add_local(globals, "print");
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

void core_init_classes()
{
    if (core_classes_inited) return;
    core_classes_inited = true;

    melon_class_object = class_new(strdup("Object"), 0, NULL);
    melon_class_class = class_new(strdup("Class"), 0, NULL);
    class_set_superclass(melon_class_class, melon_class_object);

    class_bind(melon_class_object, FROM_CSTR(strdup("classname")),
        FROM_CLOSURE(closure_new(function_native_new(object_classname))));
}

void core_free_vm()
{
    if (!core_vm_inited) return;

    function_free(core_println);
    function_free(core_print);
    closure_free(core_println_cl);
    closure_free(core_print_cl);
}

void core_free_classes()
{
    if (!core_classes_inited) return;

    class_free(melon_class_object);
    class_free(melon_class_class);
}