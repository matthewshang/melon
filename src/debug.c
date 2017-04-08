#include "debug.h"

#include <stdio.h>

const char *op_to_str(opcode op)
{
    switch (op)
    {
    case OP_RET0: return "ret0";
    case OP_NOP: return "nop";

    case OP_LOAD: return "load";
    case OP_LOADI: return "loadi";
    case OP_LOADK: return "loadk";
    case OP_STORE: return "store";

    case OP_CALL: return "call";
    case OP_JMP: return "jmp";
    case OP_LOOP: return "loop";
    case OP_JIF: return "jif";

    case OP_ADD: return "add";
    case OP_SUB: return "sub";
    case OP_MUL: return "mul";
    case OP_MOD: return "mod";
    case OP_NEG: return "neg";

    case OP_AND: return "and";
    case OP_OR: return "or";
    case OP_LT: return "lt";
    case OP_GT: return "gt";
    case OP_NOT: return "not";
    case OP_EQ: return "eq";
    case OP_NEQ: return "neq";

    case OP_HALT: return "halt";
    }
    printf("Unrecognized op %d\n", op);
    return "";
}


void disassemble_code(byte_r *code)
{
    for (int i = 0; i < vector_size(*code); i++)
    {
        uint8_t op = vector_get(*code, i);
        printf("%s", op_to_str((opcode)op));
        if (op == OP_LOADI || op == OP_STORE || op == OP_LOAD || op == OP_JIF
            || op == OP_JMP || op == OP_LOOP || op == OP_LOADK)
        {
            printf(" %d", vector_get(*code, ++i));
        }
        printf("\n");
    }
}