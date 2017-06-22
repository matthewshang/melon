#ifndef __TOKEN__
#define __TOKEN__

#include <stdbool.h>
#include <stdint.h>

#include "opcodes.h"

typedef enum
{
    TOK_OPEN_PAREN, TOK_CLOSED_PAREN, TOK_SEMICOLON, TOK_COMMA, TOK_OPEN_BRACE, 
    TOK_CLOSED_BRACE, TOK_OPEN_BRACKET, TOK_CLOSED_BRACKET, TOK_DOT,
    
    TOK_INT, TOK_FLOAT, TOK_STR, 
    
    TOK_IDENTIFIER, 
    TOK_VAR, TOK_CLASS,
    TOK_IF, TOK_ELSE, TOK_WHILE, TOK_TRUE, TOK_FALSE, TOK_FUNC, TOK_RETURN,
    TOK_STATIC, TOK_OPERATOR,

    TOK_EQ, TOK_ADDEQ, TOK_SUBEQ, TOK_MULEQ, TOK_DIVEQ,
    TOK_ADD, TOK_SUB, TOK_MUL, TOK_DIV, TOK_MOD, 
    TOK_BANG /*!*/, TOK_EQEQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LTE, TOK_GTE, TOK_AND, TOK_OR,
    
    TOK_ERROR, TOK_EOF, TOK_NONE,

    TOK_LAST
} token_type;

typedef struct
{
    token_type type;
    uint32_t offset;
    uint32_t length;
    uint32_t line;
    uint32_t col;
} token_t;

token_t token_create(token_type type, uint32_t offset, uint32_t length, uint32_t line, uint32_t col);
token_t token_error();
token_t token_none();

token_type token_punc(char c);

opcode token_to_binary_op(token_t token);
opcode token_to_unary_op(token_t token);
token_type token_op_assign_to_op(token_t token);

bool token_is_op_assign(token_t token);
const char *token_type_string(token_type token);

#endif