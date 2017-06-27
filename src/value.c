#include "value.h"

#include <stdio.h>
#include <math.h>

#include "debug.h"
#include "hash.h"
#include "opcodes.h"

void value_destroy(value_t val)
{
    if (IS_STR(val))
        string_free(AS_STR(val));
    else if (IS_CLOSURE(val))
        closure_free(AS_CLOSURE(val));
    else if (IS_CLASS(val))
        class_free(AS_CLASS(val));
    else if (IS_INSTANCE(val))
        instance_free(AS_INSTANCE(val));
    else if (IS_ARRAY(val))
        array_free(AS_ARRAY(val));
    else if (IS_RANGE(val))
        range_free(AS_RANGE(val));
}

void value_print_notag(value_t v)
{
    if (IS_BOOL(v)) printf("%s", AS_BOOL(v) == 1 ? "true" : "false");
    if (IS_INT(v)) printf("%d", AS_INT(v));
    if (IS_STR(v)) printf("%s", AS_STR(v)->s);
    if (IS_FLOAT(v)) printf("%f", AS_FLOAT(v));
    if (IS_NULL(v)) printf("{null}");
    if (IS_CLOSURE(v))
    {
        if (AS_CLOSURE(v)->f->type == FUNC_MELON)
            printf("%s", AS_CLOSURE(v)->f->identifier);
        else
            printf("{native function}");
    }
    if (IS_CLASS(v)) printf("%s", AS_CLASS(v)->identifier);
    if (IS_INSTANCE(v)) printf("{instance}");
    if (IS_ARRAY(v)) array_print(AS_ARRAY(v));
}

void value_print(value_t v)
{
    if (IS_BOOL(v)) printf("[bool]: ");
    if (IS_INT(v)) printf("[int]: ");
    if (IS_STR(v)) printf("[string]: ");
    if (IS_FLOAT(v)) printf("[float]: ");
    if (IS_CLOSURE(v)) 
    {
        if (AS_CLOSURE(v)->f->type == FUNC_MELON)
            printf("[closure]: ");
    }
    if (IS_CLASS(v)) printf("[class]: ");
    if (IS_ARRAY(v)) printf("[array]: ");
    value_print_notag(v);
    printf("\n");
}

bool value_equals(value_t v1, value_t v2)
{
    if (v1.type != v2.type) return false;

    if (IS_INT(v1) || IS_BOOL(v1))
    {
        return AS_INT(v1) == AS_INT(v2);
    }
    else if (IS_FLOAT(v1))
    {
        return AS_FLOAT(v1) == AS_FLOAT(v2);
    }
    else if (IS_STR(v1))
    {
        return strcmp(AS_STR(v1)->s, AS_STR(v2)->s) == 0;
    }  
    else if (IS_NULL(v1))
    {
        return IS_NULL(v2);
    }
    return false;
}

class_t *value_get_class(value_t v)
{
    if (v.type == melon_class_instance)
        return AS_INSTANCE(v)->c;
    if (v.type == melon_class_class && AS_CLASS(v)->metaclass)
        return AS_CLASS(v)->metaclass;
    return v.type;
}

function_t *function_native_new(melon_c_func cfunc)
{
    function_t *func = (function_t*)calloc(1, sizeof(function_t));
    func->type = FUNC_NATIVE;
    func->nupvalues = 0;
    func->cfunc = cfunc;
    return func;
}

function_t *function_new(const char *identifier)
{
    function_t *func = (function_t*)calloc(1, sizeof(function_t));
    func->type = FUNC_MELON;
    func->nupvalues = 0;
    func->identifier = identifier;
    vector_init(func->bytecode);
    vector_init(func->constpool);
    return func;
}

void function_free(function_t *func)
{
    if (!func) return;
    if (func->type == FUNC_MELON)
    {
        free(func->identifier);
        for (int i = 0; i < vector_size(func->constpool); i++)
        {
            value_t val = vector_get(func->constpool, i);
            value_destroy(val);
        }
        vector_destroy(func->constpool);
        vector_destroy(func->bytecode);
    }
    free(func);
}

value_t function_cpool_get(function_t *func, int idx)
{
    return vector_get(func->constpool, idx);
}

static void print_tabs(uint8_t ntabs)
{
    for (uint8_t i = 0; i < ntabs; i++)
    {
        printf("\t");
    }
}

static void internal_disassemble(function_t *func, uint8_t depth)
{
    if (func->type == FUNC_MELON)
    {
        print_tabs(depth); printf("disassembly of function \"%s\"\n", func->identifier);
        print_tabs(depth); printf("bytes: %d\n", vector_size(func->bytecode));

        uint32_t ninsts = 0;
        for (int i = 0; i < vector_size(func->bytecode); i++)
        {
            uint8_t op = vector_get(func->bytecode, i);
            ninsts++;
            print_tabs(depth + 1); printf("%s", op_to_str((opcode)op));
            if (op == OP_LOADI || op == OP_STOREL || op == OP_LOADL || op == OP_JIF
                || op == OP_JMP || op == OP_LOOP || op == OP_LOADK || op == OP_LOADG
                || op == OP_STOREG || op == OP_CALL || op == OP_LOADU || op == OP_STOREU
                || op == OP_NEWUP || op == OP_LOADF || op == OP_NEWARR)
            {
                printf(" %d", vector_get(func->bytecode, ++i));
            }
            if (op == OP_NEWUP)
            {
                printf(", %d", vector_get(func->bytecode, ++i));
            }
            printf(ninsts % 8 == 0 ? "\n\n" : "\n");
        }
        printf("\n");
    }
}

static void internal_class_print(class_t *c, uint8_t depth);
static void internal_cpool_dump(function_t *func, uint8_t depth);

static void debug_print_val(value_t v, uint8_t depth)
{
    if (IS_CLOSURE(v))
    {
        printf("[function] %s\n", AS_CLOSURE(v)->f->identifier);
        internal_disassemble(AS_CLOSURE(v)->f, depth + 1);
        internal_cpool_dump(AS_CLOSURE(v)->f, depth + 1);
        return;
    }
    if (IS_CLASS(v))
    {
        class_t *c = AS_CLASS(v);
        printf("[class] %s\n", c->identifier);
        internal_class_print(c, depth + 1);
        if (c->metaclass)
        {
            class_t *meta = c->metaclass;
            print_tabs(depth);
            printf("[class] %s\n", meta->identifier);
            internal_class_print(meta, depth + 1);
        }
        return;
    }
    value_print(v);
}

static void class_print_iterate(hash_entry_t *node)
{
    value_print(node->value);
    if (IS_CLOSURE(node->value))
    {
        function_disassemble(AS_CLOSURE(node->value)->f);
        function_cpool_dump(AS_CLOSURE(node->value)->f);
    }
}

void internal_class_print(class_t *c, uint8_t depth)
{
    class_t *super = c->superclass;
    print_tabs(depth); printf("nvars: %d, super: '%s'\n", c->nvars, super ? super->identifier : "none");
    hashtable_iterate(c->htable, class_print_iterate);
    //hashtable_dump(c->htable);
}

void internal_cpool_dump(function_t *func, uint8_t depth)
{
    if (func->type == FUNC_MELON)
    {
        print_tabs(depth); printf("function constants of \"%s\"\n", func->identifier);
        if (vector_size(func->constpool) == 0)
        {
            print_tabs(depth + 1); printf("none\n\n");
            return;
        }
        for (int i = 0; i < vector_size(func->constpool); i++)
        {
            value_t v = vector_get(func->constpool, i);
            print_tabs(depth + 1);
            debug_print_val(v, depth + 1);
        }
        printf("\n");
    }
}

void function_cpool_dump(function_t *func)
{
    internal_cpool_dump(func, 0);
}

void function_disassemble(function_t *func)
{
    internal_disassemble(func, 0);
}

upvalue_t *upvalue_new(value_t *value)
{
    upvalue_t *upvalue = (upvalue_t*)calloc(1, sizeof(upvalue_t));
    upvalue->value = value;
    upvalue->next = NULL;
    return upvalue;
}

void upvalue_free(upvalue_t *upvalue)
{
    free(upvalue);
}

closure_t *closure_new(function_t *func)
{
    closure_t *closure = (closure_t*)calloc(1, sizeof(closure_t));
    closure->f = func;
    closure->upvalues = NULL;
    return closure;
}

closure_t *closure_native(melon_c_func func)
{
    return closure_new(function_native_new(func));
}

void closure_free(closure_t *closure)
{
    if (closure->upvalues)
    {
        upvalue_t *upvalue = *closure->upvalues;
        while (upvalue)
        {
            upvalue_t *next = upvalue->next;
            if (upvalue->value == &upvalue->closed)
                free(upvalue);
            upvalue = next;
        }
        free(closure->upvalues);
    }
    function_free(closure->f);
    free(closure);
}

class_t *class_new(const char *identifier, uint16_t nvars, class_t *superclass)
{
    class_t *c = (class_t*)calloc(1, sizeof(class_t));
    c->superclass = superclass;
    c->identifier = identifier;
    c->nvars = nvars;
    c->htable = hashtable_new(384);
    c->meta_inited = false;
    c->static_vars = NULL;
    c->metaclass = NULL;
    return c;
}

class_t *class_new_with_meta(const char *identifier, uint16_t nvars, uint16_t nstatic, class_t *superclass)
{
    class_t *c = (class_t*)calloc(1, sizeof(class_t));
    c->identifier = identifier;
    c->superclass = superclass;
    c->nvars = nvars;
    c->htable = hashtable_new(384);
    c->meta_inited = true;
    c->static_vars = nstatic > 0 ? (value_t*)calloc(nstatic, sizeof(value_t)) : NULL;
    for (size_t i = 0; i < nstatic; i++)
    {
        c->static_vars[i] = FROM_NULL;
    }
    char buf[256];
    snprintf(buf, sizeof(buf), "%s$meta", c->identifier);
    class_t *supermeta = superclass->metaclass ? superclass->metaclass : melon_class_class;
    c->metaclass = class_new(strdup(buf), nstatic, supermeta);
    return c;
}

static void free_class_iterate(hash_entry_t *node)
{
    if (IS_STR(node->key))
    {
        string_free(AS_STR(node->key));
    }
    value_destroy(node->value);
}

void class_free(class_t *c)
{
    if (c)
    {
        if (c->identifier) free(c->identifier);
        if (c->metaclass) class_free(c->metaclass);
        if (c->static_vars) free(c->static_vars);
        hashtable_iterate(c->htable, free_class_iterate);
        hashtable_free(c->htable);
        free(c);
    }
}

void class_print(class_t *c)
{
    if (!c) return;

    internal_class_print(c, 0);
    if (c->metaclass) internal_class_print(c->metaclass, 0);
}

void class_set_superclass(class_t *c, class_t *super)
{
    c->superclass = super;
}

void class_bind(class_t *c, const char *key, value_t value)
{
    hashtable_set(c->htable, FROM_CSTR(key), value);
    c->nvars = ((hashtable_t*)c->htable)->nentrys;
}

value_t *class_lookup(class_t *c, value_t key)
{
    return hashtable_get(c->htable, key);
}

value_t *class_lookup_super(class_t *c, value_t key)
{
    class_t *current = c;
    value_t *v = NULL;
    while (current)
    {
        v = hashtable_get(current->htable, key);
        if (v) break;
        current = current->superclass;
    }

    return v;
}

closure_t *class_lookup_closure(class_t *c, value_t key)
{
    value_t *value = class_lookup(c, key);
    if (!value) return NULL;
    if (IS_CLOSURE(*value)) return AS_CLOSURE(*value);
    else return NULL;
}

instance_t *instance_new(class_t *c)
{
    instance_t *inst = (instance_t*)calloc(1, sizeof(instance_t));
    inst->c = c;
    inst->nvars = c->nvars;
    inst->vars = (value_t*)calloc(inst->nvars, sizeof(value_t));
    for (size_t i = 0; i < inst->nvars; i++)
    {
        inst->vars[i] = FROM_NULL;
    }
    return inst;
}

void instance_free(instance_t *inst)
{
    if (!inst) return;
    if (inst->vars) free(inst->vars);
    free(inst);
}

array_t *array_new()
{
    array_t *a = (array_t*)calloc(1, sizeof(array_t));
    vector_init(a->arr);
    a->size = 0;
    return a;
}

void array_free(array_t *a)
{
    vector_destroy(a->arr);
    free(a);
}

void array_push(array_t *a, value_t v)
{
    vector_push(value_t, a->arr, v);
    a->size++;
}

void array_print(array_t *a)
{
    printf("[");
    for (size_t i = 0; i < vector_size(a->arr); i++)
    {
        value_print_notag(vector_get(a->arr, i));
        if (i + 1 < vector_size(a->arr))
        {
            printf(", ");
        }
    }
    printf("]");
}

string_t *string_new(const char *s)
{
    string_t *str = (string_t*)calloc(1, sizeof(string_t));
    str->s = _strdup(s);
    str->len = strlen(s);
    str->hash = hash_string(s);
    return str;
}

void string_free(string_t *s)
{
    free(s->s);
    free(s);
}

string_t *string_copy(string_t *s)
{
    string_t *str = (string_t*)calloc(1, sizeof(string_t));
    str->s = _strdup(s->s);
    str->len = s->len;
    str->hash = s->hash;
    return str;
}

range_t *range_new(int start, int end, int step)
{
    range_t *range = (range_t*)calloc(1, sizeof(range_t));
    range->start = start;
    range->step = step;
    range->iterations = ceil((float) abs(end - start) / (float) abs(step));
    return range;
}

void range_free(range_t *range)
{
    free(range);
}
