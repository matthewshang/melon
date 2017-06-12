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
        value_print_notag(v);
    }
    printf("\n");
}

static void melon_print(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    if (nargs > 0)
    {
        value_t v = args[0];
        value_print_notag(v);
    }
}

static void object_class(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    value_t v = args[0];
    RETURN_VALUE(FROM_CLASS(value_get_class(v)));
}

static void class_name(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    value_t v = args[0];
    if (IS_CLASS(v))
    {
        RETURN_VALUE(FROM_CSTR(AS_CLASS(v)->identifier));
    }
}

static void closure_name(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    value_t v = args[0];
    if (IS_CLOSURE(v))
    {
        closure_t *cl = AS_CLOSURE(v);
        RETURN_VALUE(cl->f->type == FUNC_MELON ? FROM_CSTR(cl->f->identifier) : FROM_CSTR("{native func}"));
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

    symtable_add_local(globals, "Object");
    symtable_add_local(globals, "Class");
    symtable_add_local(globals, "Bool");
    symtable_add_local(globals, "Int");
    symtable_add_local(globals, "Float");
    symtable_add_local(globals, "String");
    symtable_add_local(globals, "Closure");
    symtable_add_local(globals, "Instance");
}

void core_register_vm(vm_t *vm)
{
    if (core_vm_inited) return;
    core_vm_inited = true;

    core_println_cl = closure_new(function_native_new(melon_println));
    vm_set_global(vm, FROM_CLOSURE(core_println_cl), PRINTLN_SLOT);

    core_print_cl = closure_new(function_native_new(melon_print));
    vm_set_global(vm, FROM_CLOSURE(core_print_cl), PRINT_SLOT);

    vm_set_global(vm, FROM_CLASS(melon_class_object), 2);
    vm_set_global(vm, FROM_CLASS(melon_class_class), 3);
    vm_set_global(vm, FROM_CLASS(melon_class_bool), 4);
    vm_set_global(vm, FROM_CLASS(melon_class_int), 5);
    vm_set_global(vm, FROM_CLASS(melon_class_float), 6);
    vm_set_global(vm, FROM_CLASS(melon_class_string), 7);
    vm_set_global(vm, FROM_CLASS(melon_class_closure), 8);
    vm_set_global(vm, FROM_CLASS(melon_class_instance), 9);
}

void core_init_classes()
{
    if (core_classes_inited) return;
    core_classes_inited = true;

    melon_class_object = class_new(strdup("Object"), 0, NULL);
    melon_class_class = class_new(strdup("Class"), 0, NULL);
    class_set_superclass(melon_class_class, melon_class_object);

    melon_class_bool = class_new_with_meta(strdup("Bool"), 0, 0, melon_class_object);
    melon_class_int = class_new_with_meta(strdup("Int"), 0, 0, melon_class_object);
    melon_class_float = class_new_with_meta(strdup("Float"), 0, 0, melon_class_object);
    melon_class_string = class_new_with_meta(strdup("String"), 0, 0, melon_class_object);
    melon_class_closure = class_new_with_meta(strdup("Closure"), 0, 0, melon_class_object);
    melon_class_instance = class_new_with_meta(strdup("Instance"), 0, 0, melon_class_object);

    class_bind(melon_class_object, FROM_STRLIT("class"), FROM_CLOSURE(closure_native(object_class)));

    class_bind(melon_class_class, FROM_STRLIT("name"), FROM_CLOSURE(closure_native(class_name)));

    class_bind(melon_class_closure, FROM_STRLIT("name"), FROM_CLOSURE(closure_native(closure_name)));
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