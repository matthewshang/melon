#include "value.h"

#include "debug.h"
#include "opcodes.h"

void value_destroy(value_t val)
{
    if (IS_STR(val))
        free(AS_STR(val));
    else if (IS_CLOSURE(val))
        closure_free(AS_CLOSURE(val));
    else if (IS_CLASS(val))
        class_free(AS_CLASS(val));
}

void value_print(value_t v)
{
    switch (v.type)
    {
    case VAL_BOOL: printf("[bool]: %s\n", AS_BOOL(v) == 1 ? "true" : "false"); break;
    case VAL_INT: printf("[int]: %d\n", AS_INT(v)); break;
    case VAL_STR: printf("[string]: %s\n", AS_STR(v)); break;
    case VAL_FLOAT: printf("[float]: %f\n", AS_FLOAT(v)); break;
    case VAL_CLOSURE: {
        if (AS_CLOSURE(v)->f->type == FUNC_MELON)
            printf("[closure]: %s\n", AS_CLOSURE(v)->f->identifier);
        else
            printf("[native function]\n");
        break;
    }
    default: break;
    }
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
            if (val.type == VAL_CLOSURE)
                function_free(val.cl->f);
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
            if (op == OP_LOADI || op == OP_STORE || op == OP_LOAD || op == OP_JIF
                || op == OP_JMP || op == OP_LOOP || op == OP_LOADK || op == OP_LOADG
                || op == OP_STOREG || op == OP_CALL || op == OP_LOADU || op == OP_STOREU
                || op == OP_NEWUP)
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
    switch (v.type)
    {
    case VAL_BOOL: printf("[bool] %s\n", AS_BOOL(v) == 1 ? "true" : "false"); break;
    case VAL_INT: printf("[int] %d\n", AS_INT(v)); break;
    case VAL_FLOAT: printf("[float] %f\n", AS_FLOAT(v)); break;
    case VAL_STR: printf("[string] %s\n", AS_STR(v)); break;
    case VAL_CLOSURE:
    {
        printf("[function] %s\n", AS_CLOSURE(v)->f->identifier);
        internal_disassemble(AS_CLOSURE(v)->f, depth + 1);
        internal_cpool_dump(AS_CLOSURE(v)->f, depth + 1);
        break;
    }
    case VAL_CLASS:
    {
        printf("[class] %s\n", AS_CLASS(v)->identifier);
        internal_class_print(AS_CLASS(v), depth + 1); break;
    }
    default: break;
    }
}

void internal_class_print(class_t *c, uint8_t depth)
{
    print_tabs(depth); printf("nvars: %d\n", c->nvars);

    for (size_t i = 0; i < c->nvars; i++)
    {
        print_tabs(depth); debug_print_val(vector_get(c->vars, i), depth);
    }
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
    free(closure);
}

class_t *class_new(const char *identifier, uint16_t nvars)
{
    class_t *c = (class_t*)calloc(1, sizeof(class_t));
    c->identifier = identifier;
    c->nvars = nvars;
    vector_init(c->vars);
    return c;
}

void class_free(class_t *c)
{
    if (c)
    {
        if (c->identifier) free(c->identifier);
        vector_destroy(c->vars);
        free(c);
    }
}

void class_print(class_t *c)
{
    if (!c) return;

    internal_class_print(c, 0);
}