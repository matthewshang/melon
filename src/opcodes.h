#ifndef __OPCODES__
#define __OPCODES__

typedef enum
{
    OP_RET0,
    OP_NOP,

    OP_LOAD,
    OP_LOADI,
    OP_LOADK,
    OP_STORE,

    OP_PRINT,
    OP_CALL,
    OP_JMP,
    OP_LOOP,
    OP_JIF,

    OP_ADD,
    OP_SUB,
    OP_MUL,
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