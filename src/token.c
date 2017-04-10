#include "token.h"

#include <stdlib.h>

token_t token_create(token_type type, int offset, int length)
{
    token_t tok;
    tok.type = type;
    tok.offset = offset;
    tok.length = length;
    return tok;
}

token_t token_error()
{
    token_t tok;
    tok.type = TOK_ERROR;
    tok.offset = -1;
    tok.length = 0;
    return tok;
}

token_type token_punc(char c)
{
    if (c == ';') return TOK_SEMICOLON;
    else if (c == '(') return TOK_OPEN_PAREN;
    else if (c == ')') return TOK_CLOSED_PAREN;
    else if (c == ',') return TOK_COMMA;
    else if (c == '{') return TOK_OPEN_BRACE;
    else if (c == '}') return TOK_CLOSED_BRACE;
    else return TOK_ERROR;
}

opcode token_to_binary_op(token_t token)
{
    switch (token.type)
    {
    case TOK_ADD: return OP_ADD;
    case TOK_SUB: return OP_SUB;
    case TOK_MUL: return OP_MUL;
    case TOK_MOD: return OP_MOD;
    case TOK_EQEQ: return OP_EQ;
    case TOK_NEQ: return OP_NEQ;
    case TOK_LT: return OP_LT;
    case TOK_GT: return OP_GT;
    case TOK_LTE: return OP_LTE;
    case TOK_GTE: return OP_GTE;
    case TOK_AND: return OP_AND;
    case TOK_OR: return OP_OR;
    }

    printf("Unrecognized token type %d\n", token.type);
    return OP_NOP;
}

opcode token_to_unary_op(token_t token)
{
    switch (token.type)
    {
    case TOK_BANG: return OP_NOT;
    case TOK_SUB: return OP_NEG;
    }

    printf("Unrecognized token type %d\n", token.type);
    return OP_NOP;
}
