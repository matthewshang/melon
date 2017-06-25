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

    NODE_UNARY, NODE_BINARY, NODE_POSTFIX, NODE_VAR, NODE_LIST, NODE_LITERAL

} node_type;

typedef enum
{
    LITERAL_BOOL, LITERAL_INT, LITERAL_FLT, LITERAL_STR
} literal_type;


typedef enum
{
    LOC_LOCAL,
    LOC_GLOBAL,
    LOC_UPVALUE,
    LOC_CLASS
} location_e;

typedef enum
{
    POST_CALL,
    POST_ACCESS,
    POST_SUBSCRIPT
} postfix_type;

typedef struct node_s
{
    node_type type;
    bool is_assign;
    token_t token;
} node_t;

typedef struct
{
    node_t base;
    node_r *stmts;

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
    node_t *init;
    node_t *cond;
    node_t *inc;
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
    token_t storage;

    location_e loc;
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
    location_e loc;
    uint32_t idx;
    node_var_decl_t *parent;
} node_func_decl_t;

typedef struct
{
    node_t base;
    const char *identifier;
    node_r *decls;

    symtable_t *symtable;
    uint8_t idx;
    location_e loc;
    uint16_t num_instvars;
    uint16_t num_staticvars;
    node_var_decl_t *constructor;
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
    postfix_type type;
    
    union
    {
        node_r *args;
        node_t *accessor;
    };
} postfix_expr_t;

typedef vector_t(postfix_expr_t*) postfix_expr_r;

typedef struct
{
    node_t base;
    node_t *target;
    
    postfix_expr_r *exprs;

} node_postfix_t;

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

    node_r *items;
} node_list_t;

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


node_t *node_block_new(node_r *stmts);
node_t *node_if_new(node_t *cond, node_t *then, node_t *els);
node_t *node_loop_new(node_t *init, node_t *cond, node_t *inc, node_t *body);
node_t *node_return_new(node_t *expr);

node_t *node_var_decl_new(token_t token, token_t storage, const char *ident, node_t *init);
node_t *node_func_decl_new(token_t token, const char *identifier, vector_t(node_var_t*) *params, node_block_t *body);
node_t *node_class_decl_new(token_t token, const char *identifier, node_r *decls);

node_t *node_binary_new(token_t op, node_t *left, node_t *right);
node_t *node_unary_new(token_t op, node_t *right);
postfix_expr_t *postfix_call_new(node_r *args);
postfix_expr_t *postfix_access_new(node_t *accessor);
postfix_expr_t *postfix_subscript_new(node_t *subscript);
node_t *node_postfix_new(node_t *target, postfix_expr_r *exprs);
node_t *node_var_new(token_t token, const char *identifier);
node_t *node_list_new(node_r *items);
node_t *node_literal_int_new(int value);
node_t *node_literal_float_new(double value);
node_t *node_literal_str_new(const char *value, int len);
node_t *node_literal_bool_new(bool value);

void ast_free(node_t *root);
void ast_print(node_t *root);

#endif