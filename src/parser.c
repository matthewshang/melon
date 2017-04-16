#include "parser.h"

#include <stdio.h>
#include <stdlib.h>

typedef vector_t(node_t*) node_r;
typedef vector_t(node_var_t*) node_var_r;

typedef enum {
    PREC_LOWEST,    // literals
    PREC_ASSIGN,    // =
    PREC_OR,        // ||
    PREC_AND,       // &&
    PREC_COMP,      // < > <= >= == !=
    PREC_TERM,      // + -
    PREC_FACTOR,    // * / %
    PREC_UNARY,     // ! -
    PREC_CALL,      // ()
} precedence_t;

typedef node_t *(*prefix_func)(lexer_t *lexer, token_t token);
typedef node_t *(*infix_func)(lexer_t *lexer, node_t *node, token_t token);

typedef struct
{
    prefix_func prefix;
    infix_func infix;
    precedence_t prec;
} parse_rule;

#define RULE(prefix, infix, prec)  (parse_rule){ prefix, infix, prec }
#define PREFIX_OP(prec)            (parse_rule){ parse_unary, NULL, prec }
#define INFIX_OP(prec)             (parse_rule){ NULL, parse_infix, prec }
#define PREFIX_RULE(prec, fn)      (parse_rule){ fn, NULL, prec }
#define INFIX_RULE(prec, fn)       (parse_rule){ NULL, fn, prec }

static parse_rule rules[TOK_LAST];
static bool rules_initialized = false;

static char *substr(const char *s, int offset, int length)
{
    char *sub = (const char*)calloc(length + 1, sizeof(char));
    sub[length] = '\0';

    for (int i = 0; i < length; i++)
    {
        sub[i] = s[offset + i];
    }

    return sub;
}

node_t *parse_expression(lexer_t *lexer);
node_t *parse_precedence(lexer_t *lexer, precedence_t prec);
node_t *parse_block(lexer_t *lexer);

static node_t *parse_num(lexer_t *lexer, token_t token)
{
    int val = strtol(lexer->source.buffer + token.offset, NULL, 10);
    return node_literal_int_new(val);
}

static node_t *parse_str(lexer_t *lexer, token_t token)
{
    char *str = substr(lexer->source.buffer, token.offset, token.length);
    return node_literal_str_new((const char*)str, token.length);
}

static node_t *parse_bool(lexer_t *lexer, token_t token)
{
    return node_literal_bool_new(token.type == TOK_TRUE);
}

static node_t *parse_identifier(lexer_t *lexer, token_t token)
{
    char *name = substr(lexer->source.buffer, token.offset, token.length);
    return node_var_new((const char*)name);
}

static node_t *parse_nested_expr(lexer_t *lexer, token_t token)
{
    node_t *expr = parse_expression(lexer);
    lexer_consume(lexer, TOK_CLOSED_PAREN, ")");
    return expr;
}

static node_t *parse_postfix(lexer_t *lexer, node_t *node, token_t token)
{
    if (lexer_match(lexer, TOK_CLOSED_PAREN))
    {
        return node_postfix_new(node, NULL);
    }

    node_r *args = (node_r*)calloc(1, sizeof(node_r));

    do
    {
        vector_push(node_t*, *args, parse_expression(lexer));
    } while (lexer_match(lexer, TOK_COMMA));
    
    lexer_consume(lexer, TOK_CLOSED_PAREN, ")");
    return node_postfix_new(node, args);
}

static node_t *parse_unary(lexer_t *lexer, token_t token)
{
    node_t *node = parse_precedence(lexer, PREC_UNARY);
    return node_unary_new(token, node);
}

static node_t *parse_infix(lexer_t *lexer, node_t *node, token_t token)
{
    //printf("parse_infix: %d %.*s\n", token.type, token.length, lexer->source.buffer + token.offset);

    node_t *right = parse_precedence(lexer, rules[token.type].prec);

    if (token_is_op_assign(token))
    {
        token.type = token_op_assign_to_op(token);
        right = node_binary_new(token, node, right);
        token.type = TOK_EQ;
    }

    return node_binary_new(token, node, right);
}

static int get_precedence(lexer_t *lexer)
{
    parse_rule *rule = &rules[lexer_peek(lexer).type];
    return rule->prec;
}

static node_t *parse_precedence(lexer_t *lexer, precedence_t prec)
{
    token_t token = lexer_advance(lexer);
    //printf("parse_prec: %d %.*s\n", token.type, token.length, lexer->source.buffer + token.offset);

    prefix_func prefix = rules[token.type].prefix;

    if (!prefix)
    {
        printf("Error: expected expression\n");
        return;
    }

    node_t *left = prefix(lexer, token);

    while (prec < get_precedence(lexer))
    {
        token = lexer_advance(lexer);

        infix_func infix = rules[token.type].infix;
        left = infix(lexer, left, token);
    }

    return left;
}

static node_t *parse_expression(lexer_t *lexer)
{
    return parse_precedence(lexer, PREC_LOWEST);
}

static void init_parse_rules()
{
    if (rules_initialized) return;

    rules[TOK_OPEN_PAREN] = RULE(parse_nested_expr, parse_postfix, PREC_CALL);

    rules[TOK_TRUE] = PREFIX_RULE(PREC_LOWEST, parse_bool);
    rules[TOK_FALSE] = PREFIX_RULE(PREC_LOWEST, parse_bool);
    rules[TOK_NUM] = PREFIX_RULE(PREC_LOWEST, parse_num);
    rules[TOK_STR] = PREFIX_RULE(PREC_LOWEST, parse_str);
    rules[TOK_IDENTIFIER] = PREFIX_RULE(PREC_LOWEST, parse_identifier);

    rules[TOK_EQ] = INFIX_OP(PREC_ASSIGN);
    rules[TOK_ADDEQ] = INFIX_OP(PREC_ASSIGN);
    rules[TOK_SUBEQ] = INFIX_OP(PREC_ASSIGN);
    rules[TOK_MULEQ] = INFIX_OP(PREC_ASSIGN);

    rules[TOK_BANG] = PREFIX_OP(PREC_UNARY);
    rules[TOK_SUB] = RULE(parse_unary, parse_infix, PREC_TERM);

    rules[TOK_AND] = INFIX_OP(PREC_AND);

    rules[TOK_OR] = INFIX_OP(PREC_OR);

    rules[TOK_EQEQ] = INFIX_OP(PREC_COMP);
    rules[TOK_NEQ] = INFIX_OP(PREC_COMP);
    rules[TOK_LT] = INFIX_OP(PREC_COMP);
    rules[TOK_GT] = INFIX_OP(PREC_COMP);
    rules[TOK_LTE] = INFIX_OP(PREC_COMP);
    rules[TOK_GTE] = INFIX_OP(PREC_COMP);

    rules[TOK_ADD] = INFIX_OP(PREC_TERM);

    rules[TOK_MUL] = INFIX_OP(PREC_FACTOR);
    rules[TOK_MOD] = INFIX_OP(PREC_FACTOR);

    rules_initialized = true;
}

static node_t *parse_if(lexer_t *lexer)
{
    node_t *cond = NULL;
    node_t *then = NULL;
    node_t *els = NULL;

    lexer_consume(lexer, TOK_OPEN_PAREN, "(");
    cond = parse_expression(lexer);
    lexer_consume(lexer, TOK_CLOSED_PAREN, ")");

    then = parse_block(lexer);

    if (lexer_match(lexer, TOK_ELSE))
    {
        // Handles else if {...
        if (lexer_match(lexer, TOK_IF)) 
        {
            els = parse_if(lexer);
        }
        else
        {
            els = parse_block(lexer);
        }
    }

    return node_if_new(cond, then, els);
}

static node_t *parse_while(lexer_t *lexer)
{
    node_t *cond = NULL;
    node_t *body = NULL;

    lexer_consume(lexer, TOK_OPEN_PAREN, "(");
    cond = parse_expression(lexer);
    lexer_consume(lexer, TOK_CLOSED_PAREN, ")");

    body = parse_block(lexer);
    return node_loop_new(cond, body);
}

static node_t *parse_return(lexer_t *lexer)
{
    node_t *expr = parse_expression(lexer);
    lexer_consume(lexer, TOK_SEMICOLON, ";");
    return node_return_new(expr);
}

static node_t *parse_expr_stmt(lexer_t *lexer)
{
    node_t *node = parse_expression(lexer);
    lexer_consume(lexer, TOK_SEMICOLON, ";");
    return node;
}

static node_t *parse_stmt(lexer_t *lexer)
{
    if (lexer_match(lexer, TOK_IF)) return parse_if(lexer);
    else if (lexer_match(lexer, TOK_WHILE)) return parse_while(lexer);
    else if (lexer_match(lexer, TOK_RETURN)) return parse_return(lexer);
    else return parse_expr_stmt(lexer);
}

static node_t *parse_var_decl(lexer_t *lexer)
{
    token_t token = lexer_consume(lexer, TOK_IDENTIFIER, "variable name");
    char *ident = substr(lexer->source.buffer, token.offset, token.length);

    node_t *init = NULL;
    if (lexer_match(lexer, TOK_EQ))
    {
        init = parse_expression(lexer);
    }

    lexer_consume(lexer, TOK_SEMICOLON, ";");

    return node_var_decl_new((const char*)ident, init);
}

static node_var_r *parse_func_params(lexer_t *lexer)
{
    if (lexer_check(lexer, TOK_CLOSED_PAREN)) return NULL;

    node_var_r *params = (node_var_r*)calloc(1, sizeof(node_var_r));

    if (!lexer_check(lexer, TOK_CLOSED_PAREN))
    {
        do
        {
            vector_push(node_var_t*, *params, (node_var_t*)parse_expression(lexer));
        } while (lexer_match(lexer, TOK_COMMA));
    }

    return params;
}

static node_t *parse_func_decl(lexer_t *lexer)
{
    token_t token = lexer_consume(lexer, TOK_IDENTIFIER, "identifier");
    char *ident = substr(lexer->source.buffer, token.offset, token.length);

    lexer_consume(lexer, TOK_OPEN_PAREN, "(");
    vector_t(node_var_t*) *params = parse_func_params(lexer);
    lexer_consume(lexer, TOK_CLOSED_PAREN, ")");

    node_t *body = parse_block(lexer);

    return node_func_decl_new((const char*)ident, params, (node_block_t*)body);
}

static node_t *parse_decl(lexer_t *lexer)
{
    if (lexer_match(lexer, TOK_VAR)) 
        return parse_var_decl(lexer);
    if (lexer_match(lexer, TOK_FUNC))
        return parse_func_decl(lexer);

    return parse_stmt(lexer);
}

static node_t *parse_block(lexer_t *lexer)
{
    node_r *stmts = (node_r*)calloc(1, sizeof(node_r));
    vector_init(*stmts);

    lexer_consume(lexer, TOK_OPEN_BRACE, "{");

    while (!lexer_check(lexer, TOK_CLOSED_BRACE))
    {
        vector_push(node_t*, *stmts, parse_decl(lexer));
    }

    lexer_consume(lexer, TOK_CLOSED_BRACE, "}");

    return node_block_new(stmts);
}

node_t *parse(lexer_t *lexer)
{
    init_parse_rules();

    node_r *stmts = (node_r*)calloc(1, sizeof(node_r));
    vector_init(*stmts);

    while (!lexer_end(lexer))
    {
        vector_push(node_t*, *stmts, parse_decl(lexer));
    }

    return node_block_new(stmts);
}