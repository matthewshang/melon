#ifndef __OPCODES__
#define __OPCODES__

typedef enum
{
    OP_RET0,
    OP_NOP,

    OP_LOAD,
    OP_LOADI,
    OP_LOADK,
    OP_LOADU,
    OP_LOADG,
    OP_STORE,
    OP_STOREU,
    OP_STOREG,

    OP_NEWUP,
    OP_CLOSURE,
    OP_CLOSE,
    OP_CALL,
    OP_JMP,
    OP_LOOP,
    OP_JIF,
    OP_RETURN,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
   
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_NEG,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE,

    OP_EQ,
    OP_NEQ,

    OP_HALT
} opcode;

#endif