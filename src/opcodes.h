#ifndef __OPCODES__
#define __OPCODES__
                        
typedef enum
{
    OP_RET0,
    OP_NOP,

    OP_LOADL,        // LOAD_LOCAL
    OP_LOADI,        // LOAD_IMPLICIT
    OP_LOADK,        // LOAD_CONSTANT
    OP_LOADU,        // LOAD_UPVALUE
    OP_LOADF,        // LOAD_FIELD
    OP_LOADG,        // LOAD_GLOBAL
    OP_STOREL,
    OP_STOREU,
    OP_STOREF,
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