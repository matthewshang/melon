#include "ast.h"

#include <stdlib.h>

#include "astwalker.h"

#define NODE_TEST(node) node
#define NODE_SETBASE(node, _type) node->base.type = _type

node_t *node_literal_int_new(int value)
{
    node_literal_t *node = (node_literal_t*)calloc(1, sizeof(node_literal_t));
    NODE_SETBASE(node, NODE_LITERAL);
    node->type = LITERAL_INT;
    node->u.i = value;
    return (node_t*)node;
}

node_t *node_literal_str_new(const char *value, int len)
{
    node_literal_t *node = (node_literal_t*)calloc(1, sizeof(node_literal_t));
    NODE_SETBASE(node, NODE_LITERAL);
    node->type = LITERAL_STR;
    node->u.s = value;
    node->str_size = len;
    return (node_t*)node;
}

node_t * node_literal_bool_new(bool value)
{
    node_literal_t *node = (node_literal_t*)calloc(1, sizeof(node_literal_t));
    NODE_SETBASE(node, NODE_LITERAL);
    node->type = LITERAL_BOOL;
    node->u.i = value;
    return (node_t*)node;
}

node_t *node_var_new(const char *identifier)
{
    node_var_t *node = (node_var_t*)calloc(1, sizeof(node_var_t));
    NODE_SETBASE(node, NODE_VAR);
    node->identifier = identifier;
    return (node_t*)node;
}

node_t *node_lambda_new(vector_t(char*) *vars, vector_t(node_t*) *body)
{
    node_lambda_t *node = (node_lambda_t*)calloc(1, sizeof(node_lambda_t));
    NODE_SETBASE(node, NODE_LAMBDA);
    node->body = body;
    node->vars = vars;
    return (node_t*)node;
}

node_t *node_call_new(const char *func, vector_t(node_t*) *args)
{
    node_call_t *node = (node_call_t*)calloc(1, sizeof(node_call_t));
    NODE_SETBASE(node, NODE_CALL);
    node->args = args;
    node->func = func;
    return (node_t*)node;
}

node_t *node_if_new(node_t *cond, node_t *then, node_t *els)
{
    node_if_t *node = (node_if_t*)calloc(1, sizeof(node_if_t));
    NODE_SETBASE(node, NODE_IF);
    node->cond = cond;
    node->then = then;
    node->els = els;
    return (node_t*)node;
}

node_t *node_loop_new(node_t *cond, node_t *body)
{
    node_loop_t *node = (node_loop_t*)calloc(1, sizeof(node_loop_t));
    NODE_SETBASE(node, NODE_LOOP);
    node->cond = cond;
    node->body = body;
    return (node_t*)node;
}

node_t *node_var_decl_new(const char *ident, node_t *init)
{
    node_var_decl_t *node = (node_var_decl_t*)calloc(1, sizeof(node_var_decl_t));
    NODE_SETBASE(node, NODE_VAR_DECL);
    node->ident = ident;
    node->init = init;
    return (node_t*)node;
}

node_t *node_binary_new(token_t op, node_t *left, node_t *right)
{
    node_binary_t *node = (node_binary_t*)calloc(1, sizeof(node_binary_t));
    NODE_SETBASE(node, NODE_BINARY);
    node->op = op;
    node->left = left;
    node->right = right;
    return (node_t*)node;
}

node_t *node_unary_new(token_t op, node_t *right)
{
    node_unary_t *node = (node_unary_t*)calloc(1, sizeof(node_unary_t));
    NODE_SETBASE(node, NODE_UNARY);
    node->op = op;
    node->right = right;
    return (node_t*)node;
}

node_t* node_prog_new(vector_t(node_t *) *stmts)
{
    node_prog_t *node = (node_prog_t*)calloc(1, sizeof(node_prog_t));
    NODE_SETBASE(node, NODE_PROG);
    node->stmts = stmts;
    return (node_t*)node;
}

static void free_node_prog(astwalker_t *self, node_prog_t *node)
{
    if (node)
    {
        if (node->stmts)
        {
            for (int i = 0; i < vector_size(*node->stmts); i++)
            {
                walk_ast(self, vector_get(*node->stmts, i));
            }
            vector_destroy(*node->stmts);
            free(node->stmts);
        }
        free(node);
    }
}

static void free_node_if(astwalker_t *self, node_if_t *node)
{
    if (node->cond) walk_ast(self, node->cond);
    if (node->then) walk_ast(self, node->then);
    if (node->els) walk_ast(self, node->els);
    free(node);
}

static void free_node_loop(astwalker_t *self, node_loop_t *node)
{
    if (node->cond) walk_ast(self, node->cond);
    if (node->body) walk_ast(self, node->body);
    free(node);
}

static void free_node_var_decl(astwalker_t *self, node_var_decl_t *node)
{
    if (node->ident) free(node->ident);
    if (node->init) walk_ast(self, node->init);
    free(node);
}

static void free_node_call(astwalker_t *self, node_call_t *node)
{
    if (node)
    {
        if (node->args)
        {
            for (int i = 0; i < vector_size(*node->args); i++)
            {
                walk_ast(self, vector_get(*node->args, i));
            }
            vector_destroy(*node->args);
            free(node->args);
        }
        if (node->func) free(node->func);
        free(node);
    }
}

static void free_node_var(astwalker_t *self, node_var_t *node)
{
    if (node->identifier) free(node->identifier);
    free(node);
}

static void free_node_literal(astwalker_t *self, node_literal_t *node)
{
    if (node)
    {
        if (node->type == LITERAL_STR) free(node->u.s);
        free(node);
    }
}

static void free_node_binary(astwalker_t *self, node_binary_t *node)
{
    if (node)
    {
        if (node->left) walk_ast(self, node->left);
        if (node->right) walk_ast(self, node->right);
        free(node);
    }
}

static void free_node_unary(astwalker_t *self, node_unary_t *node)
{
    if (node)
    {
        if (node->right) walk_ast(self, node->right);
        free(node);
    }
}

void ast_free(node_t *root)
{
    astwalker_t visitor = {
        .visit_prog = free_node_prog,
        .visit_if = free_node_if,
        .visit_loop = free_node_loop,

        .visit_var_decl = free_node_var_decl,

        .visit_binary = free_node_binary,
        .visit_unary = free_node_unary,
        .visit_call = free_node_call,
        .visit_var = free_node_var,
        .visit_literal = free_node_literal
    };
    walk_ast(&visitor, root);
}

static void print_tabs(int depth)
{
    for (int i = 0; i < depth; i++)
    {
        printf("\t");
    }
}

static void print_node_prog(astwalker_t *self, node_prog_t *node)
{
    printf("[prog] nstmts: %d\n", vector_size(*node->stmts));
    int depth = self->depth;

    for (int i = 0; i < vector_size(*node->stmts); i++)
    {
        print_tabs(depth);
        self->depth = depth + 1;
        walk_ast(self, vector_get(*node->stmts, i));
    }

    self->depth = depth;

}

static void print_node_if(astwalker_t *self, node_if_t *node)
{
    printf("[if]\n");
    int depth = self->depth;

    print_tabs(depth); printf("if-condition: ");
    self->depth = depth + 1;
    walk_ast(self, node->cond);

    print_tabs(depth); printf("if-then: ");
    self->depth = depth + 1;
    walk_ast(self, node->then);

    if (node->els)
    {
        print_tabs(depth); printf("if-else: ");
        self->depth = depth + 1;
        walk_ast(self, node->els);
    }

    self->depth = depth;
}

static void print_node_loop(astwalker_t *self, node_loop_t *node)
{
    printf("[loop]\n");
    int depth = self->depth;

    print_tabs(depth); printf("loop-condition: ");
    self->depth = depth + 1;
    walk_ast(self, node->cond);

    print_tabs(depth); printf("loop-body: ");
    self->depth = depth + 1;
    walk_ast(self, node->body);

    self->depth = depth;
}

static void print_node_var_decl(astwalker_t *self, node_var_decl_t *node)
{
    printf("[var_decl] ident: %s\n", node->ident);
    if (node->init)
    {
        int depth = self->depth;

        print_tabs(depth);
        self->depth = depth + 1;
        walk_ast(self, node->init);

        self->depth = depth;
    }
}

static void print_node_binary(astwalker_t *self, node_binary_t *node)
{
    printf("[binary] op: %d\n", node->op.type);
    int depth = self->depth;

    print_tabs(depth); 
    self->depth = depth + 1;
    walk_ast(self, node->left);

    print_tabs(depth); 
    self->depth = depth + 1;
    walk_ast(self, node->right);

    self->depth = depth;
}

static void print_node_unary(astwalker_t *self, node_unary_t *node)
{
    printf("[unary] op: %d\n", node->op.type);
    int depth = self->depth;

    print_tabs(depth);
    self->depth = depth + 1;
    walk_ast(self, node->right);

    self->depth = depth;
}

static void print_node_call(astwalker_t *self, node_call_t *node)
{
    printf("[call] nargs: %d, name: %s\n", vector_size(*node->args), node->func);
    int depth = self->depth;

    for (int i = 0; i < vector_size(*node->args); i++)
    {
        print_tabs(depth);
        self->depth = depth + 1;
        walk_ast(self, vector_get(*node->args, i));
    }

    self->depth = depth;
}

static void print_node_var(astwalker_t *self, node_var_t *node)
{
    printf("[var] name: %s\n", node->identifier);
}

static void print_node_literal(astwalker_t *self, node_literal_t *node)
{
    switch (node->type)
    {
    case LITERAL_BOOL:
    {printf("[literal] bool: %s\n", node->u.i ? "true" : "false"); break; }
    case LITERAL_INT:
    {printf("[literal] int: %d\n", node->u.i); break; }
    case LITERAL_STR:
    {printf("[literal] string: %s\n", node->u.s); break; }
    case LITERAL_FLOAT:
    {printf("[literal] float: %f\n", node->u.f); break; }
    default:
        break;
    }
}

void ast_print(node_t *root)
{
    astwalker_t visitor = {
        .depth = 1,

        .visit_prog = print_node_prog,
        .visit_if = print_node_if,
        .visit_loop = print_node_loop,

        .visit_var_decl = print_node_var_decl,

        .visit_binary = print_node_binary,
        .visit_unary = print_node_unary,
        .visit_call = print_node_call,
        .visit_var = print_node_var,
        .visit_literal = print_node_literal
    };
    walk_ast(&visitor, root);
}