#ifndef __ASTWALKER__
#define __ASTWALKER__

#include "ast.h"

typedef struct astwalker
{
    int depth;
    void *data;

    void(* visit_block)(struct astwalker *self, node_block_t *node);
    void(* visit_if)(struct astwalker *self, node_if_t *node);
    void(* visit_loop)(struct astwalker *self, node_loop_t *node);
    void(* visit_return)(struct astwalker *self, node_return_t *node);

    void(* visit_var_decl)(struct astwalker *self, node_var_decl_t *node);
    void(* visit_func_decl)(struct astwalker *self, node_func_decl_t *node);

    void(* visit_binary)(struct astwalker *self, node_binary_t *node);
    void(* visit_unary)(struct astwalker *self, node_unary_t *node);
    void(* visit_postfix)(struct astwalker *self, node_postfix_t *node);
    void(* visit_var)(struct astwalker *self, node_var_t *node);
    void(* visit_literal)(struct astwalker *self, node_literal_t *node);
} astwalker_t;

void walk_ast(astwalker_t *self, node_t *node);

#endif