#include "semantic.h"

#include "astwalker.h"
#include "symtable.h"

void visit_block(struct astwalker *self, node_block_t *node)
{
    symtable_t *saved = (symtable_t*)self->data;
    self->data = (void*)symtable_new();

    for (size_t i = 0; i < vector_size(*node->stmts); i++)
    {
        walk_ast(self, vector_get(*node->stmts, i));
    }

    node->symtable = (symtable_t*)self->data;
    self->data = (void*)saved;
}

void visit_var_decl(struct astwalker *self, node_var_decl_t *node)
{
    symtable_t *symtable = (symtable_t*)self->data;
    if (symtable_lookup(symtable, node->ident, NULL))
    {
        printf("[Error] Variable %s is already defined\n", node->ident);
        self->nerrors++;
        return;
    }
    symtable_add_local(symtable, node->ident);
}

void visit_func_decl(struct astwalker *self, node_func_decl_t *node)
{
    symtable_t *symtable = (symtable_t*)self->data;
    if (symtable_lookup(symtable, node->identifier, NULL))
    {
        printf("[Error] Function %s is already defined\n", node->identifier);
        self->nerrors++;
        return;
    }
    symtable_add_local(symtable, node->identifier);
}

bool sema_build_global_symtables(node_t *ast)
{
    astwalker_t walker = {
        .nerrors = 0,
        .depth = 0,
        .data = NULL,

        .visit_block = visit_block,
        .visit_if = NULL,
        .visit_loop = NULL,
        .visit_return = NULL,

        .visit_var_decl = visit_var_decl,
        .visit_func_decl = visit_func_decl,

        .visit_binary = NULL,
        .visit_unary = NULL,
        .visit_postfix = NULL,
        .visit_var = NULL,
        .visit_literal = NULL
    };

    walk_ast(&walker, ast);

    //symtable_dump(((node_block_t*)ast)->symtable);

    return walker.nerrors == 0;
}

bool semantic_process(node_t *ast)
{
    if (!sema_build_global_symtables(ast))
        return false;
}