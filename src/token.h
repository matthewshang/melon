#ifndef __TOKEN__
#define __TOKEN__

#include "opcodes.h"

typedef enum
{
    TOK_OPEN_PAREN, TOK_CLOSED_PAREN, TOK_SEMICOLON, TOK_COMMA, TOK_OPEN_BRACE, 
    TOK_CLOSED_BRACE,
    
    TOK_NUM, TOK_STR, TOK_TRUE, TOK_FALSE,
    
    TOK_IDENTIFIER, 
    TOK_PRINT, TOK_VAR, TOK_IF, TOK_ELSE, TOK_WHILE,

    TOK_OP, 
    TOK_EQ, TOK_ADD, TOK_SUB, TOK_MUL, TOK_MOD, TOK_BANG /*!*/, TOK_EQEQ, TOK_NEQ,
    TOK_LT, TOK_GT, TOK_LTE, TOK_GTE, TOK_AND, TOK_OR,
    
    TOK_ERROR, TOK_EOF,

    TOK_LAST
} token_type;

typedef struct
{
    token_type type;
    int offset;
    int length;
} token_t;

token_t token_create(token_type type, int offset, int length);
token_t token_error();

token_type token_punc(char c);

opcode token_to_binary_op(token_t token);
opcode token_to_unary_op(token_t token);

#endif