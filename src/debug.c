#include "debug.h"

#include <stdio.h>

const char *op_to_str(opcode op)
{
    switch (op)
    {
    case OP_RET0: return "ret0";
    case OP_NOP: return "nop";

    case OP_LOADL: return "loadl";
    case OP_LOADI: return "loadi";
    case OP_LOADK: return "loadk";
    case OP_LOADU: return "loadu";
    case OP_LOADF: return "loadf";
    case OP_LOADA: return "loada";
    case OP_LOADG: return "loadg";
    case OP_STOREL: return "storel";
    case OP_STOREU: return "storeu";
    case OP_STOREF: return "storef";
    case OP_STOREA: return "storea";
    case OP_STOREG: return "storeg";

    case OP_NEWUP: return "newup";
    case OP_CLOSURE: return "closure";
    case OP_CLOSE: return "close";
    case OP_CALL: return "call";
    case OP_JMP: return "jmp";
    case OP_LOOP: return "loop";
    case OP_JIF: return "jif";
    case OP_RETURN: return "return";

    case OP_ADD: return "add";
    case OP_SUB: return "sub";
    case OP_MUL: return "mul";
    case OP_DIV: return "div";
    case OP_MOD: return "mod";
    case OP_NEG: return "neg";

    case OP_AND: return "and";
    case OP_OR: return "or";
    case OP_LT: return "lt";
    case OP_GT: return "gt";
    case OP_LTE: return "lte";
    case OP_GTE: return "gte";
    case OP_NOT: return "not";
    case OP_EQ: return "eq";
    case OP_NEQ: return "neq";

    case OP_NEWARR: return "newarr";

    case OP_HALT: return "halt";
    }
    printf("Unrecognized op %d\n", op);
    return "";
}