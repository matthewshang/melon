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
        if (IS_BOOL(v)) printf("%s\n", AS_BOOL(v) == 1 ? "true" : "false");
        if (IS_INT(v)) printf("%d\n", AS_INT(v));
        if (IS_STR(v)) printf("%s\n", AS_STR(v));
        if (IS_FLOAT(v)) printf("%f\n", AS_FLOAT(v));
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
        if (IS_BOOL(v)) printf("%s", AS_BOOL(v) == 1 ? "true" : "false");
        if (IS_INT(v)) printf("%d", AS_INT(v));
        if (IS_STR(v)) printf("%s", AS_STR(v));
        if (IS_FLOAT(v)) printf("%f", AS_FLOAT(v));
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

    core_println_cl = closure_new(function_native_new(melon_println));
    vm_set_global(vm, FROM_CLOSURE(core_println_cl), PRINTLN_SLOT);

    core_print_cl = closure_new(function_native_new(melon_print));
    vm_set_global(vm, FROM_CLOSURE(core_print_cl), PRINT_SLOT);
}

void core_init_classes()
{
    if (core_classes_inited) return;
    core_classes_inited = true;

    melon_class_object = class_new(strdup("Object"), 0, NULL);
    melon_class_class = class_new(strdup("Class"), 0, NULL);
    class_set_superclass(melon_class_class, melon_class_object);

    melon_class_bool = class_new(strdup("Bool"), 0, NULL);
    melon_class_int = class_new(strdup("Int"), 0, NULL);
    melon_class_float = class_new(strdup("Float"), 0, NULL);
    melon_class_string = class_new(strdup("String"), 0, NULL);
    melon_class_closure = class_new(strdup("Closure"), 0, NULL);
    melon_class_instance = class_new(strdup("Instance"), 0, NULL);

    class_bind(melon_class_object, FROM_CSTR(strdup("classname")),
        FROM_CLOSURE(closure_new(function_native_new(object_classname))));
}

void core_free_vm()
{
    if (!core_vm_inited) return;

    closure_free(core_println_cl);
    closure_free(core_print_cl);
}

void core_free_classes()
{
    if (!core_classes_inited) return;

    class_free(melon_class_object);
    class_free(melon_class_class);

    class_free(melon_class_bool);
    class_free(melon_class_int);
    class_free(melon_class_float);
    class_free(melon_class_string);
    class_free(melon_class_closure);
    class_free(melon_class_instance);
}