#include "value.h"

#include "debug.h"
#include "opcodes.h"

void value_destroy(value_t val)
{
    if (val.type == VAL_STR)
        free(val.s);
    else if (val.type == VAL_CLOSURE)
        closure_free(val.cl);
}

void value_print(value_t v)
{
    switch (v.type)
    {
    case VAL_BOOL: printf("[bool]: %s\n", v.i == 1 ? "true" : "false"); break;
    case VAL_INT: printf("[int]: %d\n", v.i); break;
    case VAL_STR: printf("[string]: %s\n", v.s); break;
    case VAL_FLOAT: printf("[float]: %f\n", v.d); break;
    case VAL_CLOSURE: {
        if (v.cl->f->type == FUNC_MELON)
            printf("[closure]: %s\n", v.cl->f->identifier);
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

void function_cpool_dump(function_t *func)
{
    if (func->type == FUNC_MELON)
    {
        printf("----Dumping function constants of %s----\n", func->identifier);
        for (int i = 0; i < vector_size(func->constpool); i++)
        {
            value_t v = vector_get(func->constpool, i);
            switch (v.type)
            {
            case VAL_BOOL: printf("\t%s\n", v.i == 1 ? "true" : "false"); break;
            case VAL_INT: printf("\t%d\n", v.i); break;
            case VAL_FLOAT: printf("\t%f\n", v.d); break;
            case VAL_STR: printf("\t%s\n", v.s); break;
            case VAL_CLOSURE: function_disassemble(v.cl->f); function_cpool_dump(v.cl->f); break;
            default: break;
            }
        }
        printf("----\n\n", func->identifier);
    }
}

void function_disassemble(function_t *func)
{
    if (func->type == FUNC_MELON)
    {
        printf("----Disassembled function %s----\n", func->identifier);
        printf("Length in bytes: %d\n", vector_size(func->bytecode));
        for (int i = 0; i < vector_size(func->bytecode); i++)
        {
            uint8_t op = vector_get(func->bytecode, i);
            printf("%s", op_to_str((opcode)op));
            if (op == OP_LOADI || op == OP_STORE || op == OP_LOAD || op == OP_JIF
                || op == OP_JMP || op == OP_LOOP || op == OP_LOADK || op == OP_LOADG
                || op == OP_STOREG || op == OP_CALL || op == OP_LOADU || op == OP_STOREU
                || op == OP_NEWUP)
            {
                printf(" %d", vector_get(func->bytecode, ++i));
            }
            printf("\n");
        }
        printf("----\n\n", func->identifier);
    }
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