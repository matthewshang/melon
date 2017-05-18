#ifndef __AST__
#define __AST__

#include <stdbool.h>

#include "token.h"
#include "vector.h"

typedef struct symtable_t symtable_t;

typedef enum
{
    NODE_BLOCK, NODE_IF, NODE_LOOP, NODE_RETURN,

    NODE_VAR_DECL, NODE_FUNC_DECL, NODE_CLASS_DECL,

    NODE_UNARY, NODE_BINARY, NODE_POSTFIX, NODE_VAR, NODE_LITERAL

} node_type;

typedef enum
{
    LITERAL_BOOL, LITERAL_INT, LITERAL_FLT, LITERAL_STR
} literal_type;

typedef struct
{
    node_type type;
    bool is_assign;
    token_t token;
} node_t;

typedef struct
{
    node_t base;
    vector_t(node_t*) *stmts;

    symtable_t *symtable;
    bool is_root;
} node_block_t;

typedef struct
{
    node_t base;
    node_t *cond;
    node_t *then;
    node_t *els;
} node_if_t;

typedef struct
{
    node_t base;
    node_t *cond;
    node_t *body;
} node_loop_t;

typedef struct
{
    node_t base;
    node_t *expr;
} node_return_t;

typedef struct
{
    node_t base;
    const char *ident;
    node_t *init;

    bool is_global;
    uint8_t idx;
} node_var_decl_t;

typedef struct
{
    bool is_direct;
    uint8_t idx;
    const char *symbol;
} ast_upvalue_t;

typedef struct node_var_s node_var_t;

typedef struct
{
    node_t base;
    const char *identifier;
    vector_t(node_var_t*) *params;
    node_block_t *body;

    symtable_t *symtable;
    vector_t(ast_upvalue_t) *upvalues;
    bool is_global;
    uint32_t idx;
} node_func_decl_t;

typedef struct
{
    node_t base;
    const char *identifier;
    vector_t(node_t*) *decls;

    symtable_t *symtable;
} node_class_decl_t;

typedef struct
{
    node_t base;
    token_t op;
    node_t *left;
    node_t *right;
} node_binary_t;

typedef struct
{
    node_t base;
    token_t op;
    node_t *right;
} node_unary_t;

typedef struct
{
    node_t base;
    node_t *target;
    vector_t(node_t*) *args;
} node_postfix_t;

typedef enum
{
    LOC_LOCAL,
    LOC_GLOBAL,
    LOC_UPVALUE
} location_e;

typedef struct node_var_s
{
    node_t base;
    const char *identifier;

    uint8_t idx;
    location_e location;
} node_var_t;

typedef struct
{
    node_t base;
    literal_type type;
    int str_size;
    union
    {
        int i;
        double d;
        const char *s;
    } u;
} node_literal_t;


node_t *node_block_new(vector_t(node_t*) *stmts);
node_t *node_if_new(node_t *cond, node_t *then, node_t *els);
node_t *node_loop_new(node_t *cond, node_t *body);
node_t *node_return_new(node_t *expr);

node_t *node_var_decl_new(token_t token, const char *ident, node_t *init);
node_t *node_func_decl_new(token_t token, const char *identifier, vector_t(node_var_t*) *params, node_block_t *body);
node_t *node_class_decl_new(token_t token, const char *identifier, vector_t(node_t*) *decls);

node_t *node_binary_new(token_t op, node_t *left, node_t *right);
node_t *node_unary_new(token_t op, node_t *right);
node_t *node_postfix_new(node_t *target, vector_t(node_t*) *args);
node_t *node_var_new(token_t token, const char *identifier);
node_t *node_literal_int_new(int value);
node_t *node_literal_float_new(double value);
node_t *node_literal_str_new(const char *value, int len);
node_t *node_literal_bool_new(bool value);

void ast_free(node_t *root);
void ast_print(node_t *root);

#endif