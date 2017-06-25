#include "ast.h"

#include <stdlib.h>

#include "astwalker.h"
#include "symtable.h"

#define NODE_SETBASE(node, _type) node->base.type = _type

node_t* node_block_new(node_r *stmts)
{
    node_block_t *node = (node_block_t*)calloc(1, sizeof(node_block_t));
    NODE_SETBASE(node, NODE_BLOCK);
    node->stmts = stmts;

    node->symtable = NULL;
    node->is_root = false;
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

node_t *node_loop_while_new(node_t *cond, node_t *body)
{
    node_loop_t *node = (node_loop_t*)calloc(1, sizeof(node_loop_t));
    NODE_SETBASE(node, NODE_LOOP);
    node->type = LOOP_WHILE;
    node->init = NULL;
    node->cond = cond;
    node->inc = NULL;
    node->body = body;
    return (node_t*)node;
}

node_t *node_loop_cfor_new(node_t *init, node_t *cond, node_t *inc, node_t *body)
{
    node_loop_t *node = (node_loop_t*)calloc(1, sizeof(node_loop_t));
    NODE_SETBASE(node, NODE_LOOP);
    node->type = LOOP_CFOR;
    node->init = init;
    node->cond = cond;
    node->inc = inc;
    node->body = body;
    return (node_t*)node;
}

node_t *node_loop_forin_new(node_t *init, node_t *target, node_t *body)
{
    node_loop_t *node = (node_loop_t*)calloc(1, sizeof(node_loop_t));
    NODE_SETBASE(node, NODE_LOOP);
    node->type = LOOP_FORIN;
    node->init = init;
    node->cond = target;
    node->body = body;
    return (node_t*)node;
}

node_t *node_return_new(node_t *expr)
{
    node_return_t *node = (node_return_t*)calloc(1, sizeof(node_return_t));
    NODE_SETBASE(node, NODE_RETURN);
    node->expr = expr;
    return (node_t*)node;
}

node_t *node_var_decl_new(token_t token, token_t storage, const char *ident, node_t *init)
{
    node_var_decl_t *node = (node_var_decl_t*)calloc(1, sizeof(node_var_decl_t));
    NODE_SETBASE(node, NODE_VAR_DECL);
    node->base.token = token;

    node->storage = storage;
    node->ident = ident;
    node->init = init;
    return (node_t*)node;
}

node_t *node_func_decl_new(token_t token, const char *identifier, vector_t(node_var_t*) *params, node_block_t *body)
{
    node_func_decl_t *node = (node_func_decl_t*)calloc(1, sizeof(node_func_decl_t));
    NODE_SETBASE(node, NODE_FUNC_DECL);
    node->base.token = token;

    node->identifier = identifier;
    node->body = body;
    node->params = params;

    node->upvalues = (vector_t(ast_upvalue_t)*)calloc(1, sizeof(*node->upvalues));
    vector_init(*node->upvalues);
    node->parent = NULL;
    return (node_t*)node;
}

node_t *node_class_decl_new(token_t token, const char *identifier, node_r *decls)
{
    node_class_decl_t *node = (node_class_decl_t*)calloc(1, sizeof(node_class_decl_t));
    NODE_SETBASE(node, NODE_CLASS_DECL);
    node->base.token = token;

    node->identifier = identifier;
    node->decls = decls;

    node->num_instvars = 0;
    node->num_staticvars = 0;
    node->constructor = NULL;
    return (node_t*)node;
}

node_t *node_binary_new(token_t op, node_t *left, node_t *right)
{
    node_binary_t *node = (node_binary_t*)calloc(1, sizeof(node_binary_t));
    NODE_SETBASE(node, NODE_BINARY);
    node->base.token = op;

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

postfix_expr_t *postfix_call_new(node_r *args)
{
    postfix_expr_t *expr = (postfix_expr_t*)calloc(1, sizeof(postfix_expr_t));
    expr->type = POST_CALL;
    expr->args = args;
    return expr;
}

postfix_expr_t *postfix_access_new(node_t *accessor)
{
    postfix_expr_t *expr = (postfix_expr_t*)calloc(1, sizeof(postfix_expr_t));
    expr->type = POST_ACCESS;
    expr->accessor = accessor;
    return expr;
}

postfix_expr_t *postfix_subscript_new(node_t *subscript)
{
    postfix_expr_t *expr = (postfix_expr_t*)calloc(1, sizeof(postfix_expr_t));
    expr->type = POST_SUBSCRIPT;
    expr->accessor = subscript;
    return expr;
}

node_t *node_postfix_new(node_t *target, postfix_expr_r *exprs)
{
    node_postfix_t *node = (node_postfix_t*)calloc(1, sizeof(node_postfix_t));
    NODE_SETBASE(node, NODE_POSTFIX);
    node->exprs = exprs;
    node->target = target;
    return (node_t*)node;
}

node_t *node_var_new(token_t token, const char *identifier)
{
    node_var_t *node = (node_var_t*)calloc(1, sizeof(node_var_t));
    NODE_SETBASE(node, NODE_VAR);
    node->base.token = token;
    node->identifier = identifier;
    return (node_t*)node;
}

node_t *node_list_new(node_r *items)
{
    node_list_t *node = (node_list_t*)calloc(1, sizeof(node_list_t));
    NODE_SETBASE(node, NODE_LIST);
    node->items = items;
    return (node_t*)node;
}

node_t *node_literal_int_new(int value)
{
    node_literal_t *node = (node_literal_t*)calloc(1, sizeof(node_literal_t));
    NODE_SETBASE(node, NODE_LITERAL);
    node->type = LITERAL_INT;
    node->u.i = value;
    return (node_t*)node;
}

node_t *node_literal_float_new(double value)
{
    node_literal_t *node = (node_literal_t*)calloc(1, sizeof(node_literal_t));
    NODE_SETBASE(node, NODE_LITERAL);
    node->type = LITERAL_FLT;
    node->u.d = value;
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

static void free_node_block(astwalker_t *self, node_block_t *node)
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
    if (node->is_root && node->symtable) symtable_free(node->symtable);
    free(node);
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
    if (node->init) walk_ast(self, node->init);
    if (node->cond) walk_ast(self, node->cond);
    if (node->inc) walk_ast(self, node->inc);
    if (node->body) walk_ast(self, node->body);
    free(node);
}

static void free_node_return(astwalker_t *self, node_return_t *node)
{
    if (node->expr) walk_ast(self, node->expr);
    free(node);
}

static void free_node_var_decl(astwalker_t *self, node_var_decl_t *node)
{
    if (node->ident) free(node->ident);
    if (node->init) walk_ast(self, node->init);
    free(node);
}

static void free_node_func_decl(astwalker_t *self, node_func_decl_t *node)
{
    if (node->body) walk_ast(self, node->body);
    if (node->params)
    {
        for (int i = 0; i < vector_size(*node->params); i++)
        {
            walk_ast(self, vector_get(*node->params, i));
        }
        vector_destroy(*node->params);
        free(node->params);
    }
    if (node->upvalues)
    {
        vector_destroy(*node->upvalues);
        free(node->upvalues);
    }
    if (node->symtable) symtable_free(node->symtable);

    free(node);
}

static void free_node_class_decl(astwalker_t *self, node_class_decl_t *node)
{
    if (node->identifier) free(node->identifier);
    if (node->symtable) symtable_free(node->symtable);
    if (node->decls)
    {
        for (size_t i = 0; i < vector_size(*node->decls); i++)
        {
            walk_ast(self, vector_get(*node->decls, i));
        }
        vector_destroy(*node->decls);
        free(node->decls);
    }

    free(node);
}

static void free_node_binary(astwalker_t *self, node_binary_t *node)
{
    if (node->left) walk_ast(self, node->left);
    if (node->right) walk_ast(self, node->right);
    free(node);
}

static void free_node_unary(astwalker_t *self, node_unary_t *node)
{
    if (node->right) walk_ast(self, node->right);
    free(node);
}

static void free_node_postfix(astwalker_t *self, node_postfix_t *node)
{
    if (node->exprs)
    {
        for (size_t i = 0; i < vector_size(*node->exprs); i++)
        {
            postfix_expr_t *expr = vector_get(*node->exprs, i);
            if (expr->type == POST_CALL && expr->args)
            {
                for (size_t j = 0; j < vector_size(*expr->args); j++)
                {
                    walk_ast(self, vector_get(*expr->args, j));
                }
                vector_destroy(*expr->args);
                free(expr->args);
            }
            else if (expr->type == POST_ACCESS || expr->type == POST_SUBSCRIPT)
            {
                walk_ast(self, expr->accessor);
            }
        }
        vector_destroy(*node->exprs);
        free(node->exprs);
    }
    if (node->target) free(node->target);
    free(node);
}

static void free_node_var(astwalker_t *self, node_var_t *node)
{
    if (node->identifier) free(node->identifier);
    free(node);
}

static void free_node_list(astwalker_t *self, node_list_t *node)
{
    if (node->items)
    {
        for (size_t i = 0; i < vector_size(*node->items); i++)
        {
            walk_ast(self, vector_get(*node->items, i));
        }
        vector_destroy(*node->items);
        free(node->items);
    }
    free(node);
}

static void free_node_literal(astwalker_t *self, node_literal_t *node)
{
    if (node->type == LITERAL_STR) free(node->u.s);
    free(node);
}

void ast_free(node_t *root)
{
    astwalker_t visitor = {
        .visit_block = free_node_block,
        .visit_if = free_node_if,
        .visit_loop = free_node_loop,
        .visit_return = free_node_return,

        .visit_var_decl = free_node_var_decl,
        .visit_func_decl = free_node_func_decl,
        .visit_class_decl = free_node_class_decl,

        .visit_binary = free_node_binary,
        .visit_unary = free_node_unary,
        .visit_postfix = free_node_postfix,
        .visit_var = free_node_var,
        .visit_list = free_node_list,
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

static void print_node_block(astwalker_t *self, node_block_t *node)
{
    printf("[block] nstmts: %d\n", vector_size(*node->stmts));
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
    printf("[loop] type: %d\n", node->type);
    int depth = self->depth;

    if (node->init)
    {
        print_tabs(depth); printf("loop-init: ");
        self->depth = depth + 1;
        walk_ast(self, node->init);
    }

    print_tabs(depth); printf("loop-condition: ");
    self->depth = depth + 1;
    walk_ast(self, node->cond);

    if (node->inc)
    {
        print_tabs(depth); printf("loop-inc: ");
        self->depth = depth + 1;
        walk_ast(self, node->inc);
    }

    print_tabs(depth); printf("loop-body: ");
    self->depth = depth + 1;
    walk_ast(self, node->body);

    self->depth = depth;
}

static void print_node_return(astwalker_t *self, node_return_t *node)
{
    printf("[return]: ");
    int depth = self->depth;

    self->depth = depth + 1;
    walk_ast(self, node->expr);

    self->depth = depth;
}

static void print_node_var_decl(astwalker_t *self, node_var_decl_t *node)
{
    printf("[var_decl] ident: %s", node->ident);
    if (node->storage.type != TOK_NONE)
    {
        printf(", storage: %s\n", token_type_string(node->storage.type));
    }
    else
    {
        printf("\n");
    }
    if (node->init)
    {
        int depth = self->depth;

        print_tabs(depth); printf("var-init: ");
        self->depth = depth + 1;
        walk_ast(self, node->init);

        self->depth = depth;
    }
}

static void print_node_func_decl(astwalker_t *self, node_func_decl_t *node)
{
    printf("[func_decl] ident: %s\n", node->identifier);
    int depth = self->depth;

    if (node->params)
    {
        print_tabs(depth); printf("func-params: ");
        int param_size = vector_size(*node->params);
        for (int i = 0; i < param_size; i++)
        {
            node_var_t *var = vector_get(*node->params, i);
            printf("%s", var->identifier);
            if (i < param_size - 1) printf(", ");
        }
        printf("\n");
    }

    print_tabs(depth); printf("func-body: ");
    self->depth = depth + 1;
    walk_ast(self, node->body);

    self->depth = depth;
}

static void print_node_class_decl(astwalker_t *self, node_class_decl_t *node)
{
    printf("[class_decl] ident: %s\n", node->identifier);
    int depth = self->depth;

    if (node->decls)
    {
        print_tabs(depth); printf("class-decls:\n");
        for (int i = 0; i < vector_size(*node->decls); i++)
        {
            print_tabs(depth);
            self->depth = depth + 1;
            walk_ast(self, vector_get(*node->decls, i));
        }
    }

    self->depth = depth;
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

static void print_node_postfix(astwalker_t *self, node_postfix_t *node)
{
    int depth = self->depth;

    printf("[postfix] target: "); 
    
    self->depth = depth + 1;
    walk_ast(self, node->target);

    if (node->exprs)
    {
        print_tabs(depth);
        printf("postfix-exprs (%d):\n", vector_size(*node->exprs));
        for (size_t i = 0; i < vector_size(*node->exprs); i++)
        {
            postfix_expr_t *expr = vector_get(*node->exprs, i);
            print_tabs(depth + 1);
            if (expr->type == POST_CALL)
            {
                if (!expr->args)
                {
                    printf("[post-call] args: 0\n");
                    continue;
                }
                printf("[post-call] args: %d\n", vector_size(*expr->args));
                for (size_t j = 0; j < vector_size(*expr->args); j++)
                {
                    print_tabs(depth + 2);
                    self->depth = depth + 3;
                    walk_ast(self, vector_get(*expr->args, j));
                }
            }
            else if (expr->type == POST_ACCESS)
            {
                printf("[post-access]: ");
                self->depth = depth + 1;
                walk_ast(self, expr->accessor);
            }
            else if (expr->type == POST_SUBSCRIPT)
            {
                printf("[post-subscript]: ");
                self->depth = depth + 1;
                walk_ast(self, expr->accessor);
            }
        }
    }

    self->depth = depth;
}

static void print_node_var(astwalker_t *self, node_var_t *node)
{
    printf("[var] name: %s\n", node->identifier);
}

static void print_node_list(astwalker_t *self, node_list_t *node)
{
    printf("[list] nitems: %d\n", vector_size(*node->items));
    int depth = self->depth;

    for (int i = 0; i < vector_size(*node->items); i++)
    {
        print_tabs(depth);
        self->depth = depth + 1;
        walk_ast(self, vector_get(*node->items, i));
    }

    self->depth = depth;
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
    case LITERAL_FLT:
    {printf("[literal] float: %f\n", node->u.d); break; }
    default:
        break;
    }
}

void ast_print(node_t *root)
{
    astwalker_t visitor = {
        .depth = 1,

        .visit_block = print_node_block,
        .visit_if = print_node_if,
        .visit_loop = print_node_loop,
        .visit_return = print_node_return,

        .visit_var_decl = print_node_var_decl,
        .visit_func_decl = print_node_func_decl,
        .visit_class_decl = print_node_class_decl,

        .visit_binary = print_node_binary,
        .visit_unary = print_node_unary,
        .visit_postfix = print_node_postfix,
        .visit_var = print_node_var,
        .visit_list = print_node_list,
        .visit_literal = print_node_literal
    };
    walk_ast(&visitor, root);
}