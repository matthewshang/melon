#include "value.h"

#include "debug.h"
#include "opcodes.h"

void value_destroy(value_t val)
{
    if (val.type == VAL_STR)
        free(val.s);
    else if (val.type == VAL_FUNC)
        function_free(val.fn);
}

function_t *function_native_new(melon_c_func cfunc)
{
    function_t *func = (function_t*)calloc(1, sizeof(function_t));
    func->type = FUNC_NATIVE;
    func->cfunc = cfunc;
    return func;
}

function_t *function_new(const char *identifier)
{
    function_t *func = (function_t*)calloc(1, sizeof(function_t));
    func->type = FUNC_MELON;
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
            value_destroy(vector_get(func->constpool, i));
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
            case VAL_STR: printf("\t%s\n", v.s); break;
            case VAL_FUNC: function_disassemble(v.fn); function_cpool_dump(v.fn); break;
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
                || op == OP_STOREG || op == OP_CALL)
            {
                printf(" %d", vector_get(func->bytecode, ++i));
            }
            printf("\n");
        }
        printf("----\n\n", func->identifier);
    }
}