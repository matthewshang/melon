#include "token.h"

#include <stdlib.h>

token_t token_create(token_type type, uint32_t offset, uint32_t length, uint32_t line, uint32_t col)
{
    token_t tok;
    tok.type = type;
    tok.offset = offset;
    tok.length = length;
    tok.line = line;
    tok.col = col;
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

token_t token_none()
{
    return token_create(TOK_NONE, 0, 0, 0, 0);
}

token_type token_punc(char c)
{
    if (c == ';') return TOK_SEMICOLON;
    else if (c == '(') return TOK_OPEN_PAREN;
    else if (c == ')') return TOK_CLOSED_PAREN;
    else if (c == ',') return TOK_COMMA;
    else if (c == '.') return TOK_DOT;
    else if (c == '{') return TOK_OPEN_BRACE;
    else if (c == '}') return TOK_CLOSED_BRACE;
    else if (c == '[') return TOK_OPEN_BRACKET;
    else if (c == ']') return TOK_CLOSED_BRACKET;
    else return TOK_ERROR;
}

opcode token_to_binary_op(token_t token)
{
    switch (token.type)
    {
    case TOK_ADD: return OP_ADD;
    case TOK_SUB: return OP_SUB;
    case TOK_MUL: return OP_MUL;
    case TOK_DIV: return OP_DIV;
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

token_type token_op_assign_to_op(token_t token)
{
    switch (token.type)
    {
    case TOK_ADDEQ: return TOK_ADD;
    case TOK_SUBEQ: return TOK_SUB;
    case TOK_MULEQ: return TOK_MUL;
    case TOK_DIVEQ: return TOK_DIV;
    default: printf("Token %d is not of op assign type\n", token.type); break;
    }

    return TOK_ERROR;
}

bool token_is_op_assign(token_t token)
{
    return token.type > TOK_EQ && token.type <= TOK_DIVEQ;
}

const char *token_type_string(token_type type)
{
    switch (type)
    {
    case TOK_OPEN_PAREN: return "(";
    case TOK_CLOSED_PAREN: return ")";
    case TOK_SEMICOLON: return ";";
    case TOK_COMMA: return ",";
    case TOK_DOT: return ".";
    case TOK_OPEN_BRACE: return "{";
    case TOK_CLOSED_BRACE: return "}";
    case TOK_OPEN_BRACKET: return "[";
    case TOK_CLOSED_BRACKET: return "]";
    case TOK_RANGE: return "..";

    case TOK_INT: return "int";
    case TOK_FLOAT: return "float";
    case TOK_STR: return "string";

    case TOK_IDENTIFIER: return "identifier";
    case TOK_VAR: return "var";
    case TOK_CLASS: return "class";
    case TOK_IF: return "if";
    case TOK_ELSE: return "else";
    case TOK_WHILE: return "while";
    case TOK_FOR: return "for";
    case TOK_IN: return "in";
    case TOK_TRUE: return "true";
    case TOK_FALSE: return "false";
    case TOK_FUNC: return "func";
    case TOK_RETURN: return "return";
    case TOK_STATIC: return "static";
    case TOK_OPERATOR: return "operator";
    default: return "token";
    }
}
