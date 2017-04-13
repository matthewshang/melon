#include "symtable.h"

#include <string.h>
#include <stdio.h>

symtable_t *symtable_new()
{
    symtable_t *table = (symtable_t*)calloc(1, sizeof(symtable_t));
    vector_init(table->stack);
    table->top = 0;
    decl_r *first_scope = (decl_r*)calloc(1, sizeof(decl_r));
    vector_init(*first_scope);
    vector_push(decl_r*, table->stack, first_scope);
    return table;
}

void symtable_free(symtable_t *table)
{
    for (size_t i = 0; i <= table->top; i++)
    {
        vector_destroy(*vector_get(table->stack, i));
        free(vector_get(table->stack, i));
    }
    vector_destroy(table->stack);
    free(table);
}

node_t *symtable_lookup(symtable_t *table, const char *symbol)
{
    for (int j = table->top; j >= 0; j--)
    {
        decl_r *scope = vector_get(table->stack, j);
        for (size_t i = 0; i < vector_size(*scope); i++)
        {
            decl_t decl = vector_get(*scope, i);
            if (strcmp(symbol, decl.identifier) == 0) return decl.node;
        }
    }
    return NULL;
}

int symtable_add_local(symtable_t *table, const char *symbol, node_t *node)
{
    node_t *check = symtable_lookup(table, symbol);
    if (check)
    {
        printf("Symbol %s already defined\n", symbol);
        if (node->type == NODE_VAR_DECL) return ((node_var_decl_t*)node)->idx;
        else if (node->type == NODE_FUNC_DECL) return ((node_func_decl_t*)node)->idx;
        else return -1;
    }

    decl_r *scope = vector_get(table->stack, table->top);
    vector_push(decl_t, *scope, ((decl_t) { .identifier = symbol, .node = node }));
    return vector_size(*scope) - 1;
}

void symtable_enter_scope(symtable_t *table)
{
    table->top++;
    decl_r *new_scope = (decl_r*)calloc(1, sizeof(decl_r));
    vector_init(*new_scope);
    vector_push(decl_r*, table->stack, new_scope);
}

void symtable_exit_scope(symtable_t *table)
{
    decl_r *scope = vector_get(table->stack, table->top);
    vector_destroy(*scope);
    free(scope);
    vector_pop(table->stack);
    table->top--;
}

bool symtable_is_global(symtable_t *table)
{
    return table->top == 0;
}

void symtable_dump(symtable_t *table)
{
    printf("----Dumping symtable----\n");
    decl_r *scope = vector_get(table->stack, table->top);
    for (size_t i = 0; i < vector_size(*scope); i++)
    {
        printf("%s\n", vector_get(*scope, i).identifier);
    }
    printf("----\n");
}