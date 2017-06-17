#include "core.h"

#include <stdbool.h>
#include <stdio.h>

#include "ast.h"
#include "value.h"

#define PRINTLN_SLOT 0
#define PRINT_SLOT   1
#define RETURN_VALUE(val)                                                               \
        do {                                                                            \
            vm_set_stack(vm, val, retidx);                                              \
            return true;                                                                \
        } while (0)

#define RETURN                                                                     \
        do {                                                                            \
            return true;                                                                \
        } while (0)

#define RUNTIME_ERROR(...)                                                              \
        do {                                                                            \
            printf("Runtime error: ");                                                  \
            printf(__VA_ARGS__);                                                        \
            return false;                                                               \
        } while (0)


static bool melon_println(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    if (nargs > 0)
    {
        value_t v = args[0];
        value_print_notag(v);
    }
    printf("\n");
    RETURN;
}

static bool melon_print(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    if (nargs > 0)
    {
        value_t v = args[0];
        value_print_notag(v);
    }
    RETURN;
}

static bool object_class(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    value_t v = args[0];
    RETURN_VALUE(FROM_CLASS(value_get_class(v)));
}

static bool object_loadfield(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    value_t object = args[0];
    value_t accessor = args[1];
    value_t *index = NULL;
    if (IS_INT(accessor))
    {
        index = &accessor;
    }
    else
    {
        index = class_lookup_super(value_get_class(object), accessor);
    }

    if (!index)
    {
        RUNTIME_ERROR("class %s does not have property %s\n",
            value_get_class(object)->identifier, AS_STR(accessor));
    }

    if (IS_INT(*index))
    {
        if (IS_CLASS(object))
            RETURN_VALUE(AS_CLASS(object)->static_vars[AS_INT(*index)]);
        else if (IS_INSTANCE(object))
            RETURN_VALUE(AS_INSTANCE(object)->vars[AS_INT(*index)]);
        else
            RUNTIME_ERROR("tried to access an instance variable of non-instance object\n");
    }
    else
    {
        RETURN_VALUE(*index);
    }
}

static bool object_storefield(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    value_t tostore = args[0];
    value_t object = args[1];
    value_t accessor = args[2];
    value_t *index;
    if (IS_INT(accessor))
    {
        index = &accessor;
    }
    else
    {
        index = class_lookup_super(value_get_class(object), accessor);
    }

    if (!index)
    {
        RUNTIME_ERROR("class %s does not have property %s\n",
            value_get_class(object)->identifier, AS_STR(accessor));
    }

    if (IS_CLASS(object))
    {
        AS_CLASS(object)->static_vars[AS_INT(*index)] = tostore;
    }
    else if (IS_INSTANCE(object))
    {
        AS_INSTANCE(object)->vars[AS_INT(*index)] = tostore;
    }
    else
    {
        RUNTIME_ERROR("tried to modify an instance variable of non-instance object\n");
    }
    RETURN;
}

static bool class_name(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    value_t v = args[0];
    if (IS_CLASS(v))
    {
        RETURN_VALUE(FROM_CSTR(AS_CLASS(v)->identifier));
    }
}

static bool closure_name(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    value_t v = args[0];
    if (IS_CLOSURE(v))
    {
        closure_t *cl = AS_CLOSURE(v);
        RETURN_VALUE(cl->f->type == FUNC_MELON ? FROM_CSTR(cl->f->identifier) : FROM_CSTR("{native func}"));
    }
}

static bool array_new_inst(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    array_t *a = array_new();
    value_t v = FROM_ARRAY(a);
    vm_push_mem(vm, v);

    if (nargs > 0)
    {
        vector_realloc(value_t, a->arr, AS_INT(args[0]));
        a->arr.n = AS_INT(args[0]);
    }
    RETURN_VALUE(v);
}

static bool array_loadat(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    if (!IS_INT(args[1]))
    {
        RUNTIME_ERROR("Array accessor must be an int\n");
    }
    array_t *object = AS_ARRAY(args[0]);
    int accessor = AS_INT(args[1]);

    if (accessor >= vector_size(object->arr))
    {
        RUNTIME_ERROR("Array accessor out of bounds\n");
    }

    value_t v = vector_get(object->arr, accessor);
    RETURN_VALUE(v);
}

static bool array_storeat(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    if (!IS_INT(args[2]))
    {
        RUNTIME_ERROR("Array accessor must be an int\n");
    }
    value_t tostore = args[0];
    array_t *object = AS_ARRAY(args[1]);
    int accessor = AS_INT(args[2]);
    
    if (accessor >= vector_size(object->arr))
    {
        RUNTIME_ERROR("Array accessor out of bounds\n");
    }

    vector_set(object->arr, accessor, tostore);
    RETURN;
}

static bool array_size(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    array_t *arr = AS_ARRAY(args[0]);
    RETURN_VALUE(FROM_INT(vector_size(arr->arr)));
}

static bool array_add(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    array_t *arr = AS_ARRAY(args[0]);
    vector_push(value_t, arr->arr, args[1]);
    RETURN;
}

static bool array_map(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx)
{
    if (!IS_CLOSURE(args[1]))
        RUNTIME_ERROR("array_map: argument must be a closure\n");
    array_t *arr = AS_ARRAY(args[0]);
    array_t *new_arr = array_new();
    bool new_elements = false;

    closure_t *cl = AS_CLOSURE(args[1]);
    value_t cl_args[1];
    value_t *ret = NULL;
    for (size_t i = 0; i < vector_size(arr->arr); i++)
    {
        value_t v = vector_get(arr->arr, i);
        cl_args[0] = v;
        vm_run_closure(vm, cl, cl_args, 1, &ret);
        if (ret)
        {
            new_elements = true;
            vector_push(value_t, new_arr->arr, *ret);
        }
    }

    if (new_elements)
    {
        value_t arr_val = FROM_ARRAY(new_arr);
        vm_push_mem(vm, arr_val);
        RETURN_VALUE(arr_val);
    }
    else
    {
        array_free(new_arr);
        RETURN_VALUE(args[0]);
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
    symtable_add_local(globals, "Array");
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
    vm_set_global(vm, FROM_CLASS(melon_class_array), 10);
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
    melon_class_array = class_new_with_meta(strdup("Array"), 0, 0, melon_class_object);

    class_bind(melon_class_object, FROM_STRLIT("class"), FROM_CLOSURE(closure_native(object_class)));
    class_bind(melon_class_object, FROM_STRLIT("$loadfield"), FROM_CLOSURE(closure_native(object_loadfield)));
    class_bind(melon_class_object, FROM_STRLIT("$storefield"), FROM_CLOSURE(closure_native(object_storefield)));

    class_bind(melon_class_class, FROM_STRLIT("name"), FROM_CLOSURE(closure_native(class_name)));

    class_bind(melon_class_closure, FROM_STRLIT("name"), FROM_CLOSURE(closure_native(closure_name)));

    class_bind(melon_class_array, FROM_STRLIT("$loadat"), FROM_CLOSURE(closure_native(array_loadat)));
    class_bind(melon_class_array, FROM_STRLIT("$storeat"), FROM_CLOSURE(closure_native(array_storeat)));
    class_bind(melon_class_array, FROM_STRLIT("size"), FROM_CLOSURE(closure_native(array_size)));
    class_bind(melon_class_array, FROM_STRLIT("add"), FROM_CLOSURE(closure_native(array_add)));
    class_bind(melon_class_array, FROM_STRLIT("get"), FROM_CLOSURE(closure_native(array_loadat)));
    class_bind(melon_class_array, FROM_STRLIT("map"), FROM_CLOSURE(closure_native(array_map)));

    class_t *array_meta = melon_class_array->metaclass;
    class_bind(array_meta, FROM_STRLIT("$new"), FROM_CLOSURE(closure_native(array_new_inst)));
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
    class_free(melon_class_array);
}