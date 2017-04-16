#include "symtable.h"

#include <string.h>
#include <stdio.h>

symtable_t *symtable_new()
{
    symtable_t *table = (symtable_t*)calloc(1, sizeof(symtable_t));
    vector_init(table->stack);
    table->top = 0;
    symtable_entry_r *first_scope = (symtable_entry_r*)calloc(1, sizeof(symtable_entry_r));
    vector_init(*first_scope);
    vector_push(symtable_entry_r*, table->stack, first_scope);
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

bool symtable_lookup(symtable_t *table, const char *symbol, decl_info_t *ret)
{
    for (int j = table->top; j >= 0; j--)
    {
        symtable_entry_r *scope = vector_get(table->stack, j);
        for (size_t i = 0; i < vector_size(*scope); i++)
        {
            symtable_entry_t entry = vector_get(*scope, i);
            if (strcmp(symbol, entry.identifier) == 0)
            {
                if (ret) *ret = entry.decl;
                return true;
            }
        }
    }
    return false;
}

uint8_t symtable_add_local(symtable_t *table, const char *symbol)
{
    decl_info_t decl;
    if (symtable_lookup(table, symbol, &decl))
    {
        printf("Symbol %s already defined\n", symbol);
        return decl.idx;
    }

    symtable_entry_r *scope = vector_get(table->stack, table->top);
    decl.is_global = symtable_is_global(table);
    decl.idx = vector_size(*scope);
    vector_push(symtable_entry_t, *scope, ((symtable_entry_t){ .identifier = symbol, .decl = decl }));
    return decl.idx;
}

void symtable_enter_scope(symtable_t *table)
{
    table->top++;
    symtable_entry_r *new_scope = (symtable_entry_r*)calloc(1, sizeof(symtable_entry_r));
    vector_init(*new_scope);
    vector_push(symtable_entry_r*, table->stack, new_scope);
}

void symtable_exit_scope(symtable_t *table)
{
    symtable_entry_r *scope = vector_get(table->stack, table->top);
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
    symtable_entry_r *scope = vector_get(table->stack, table->top);
    for (size_t i = 0; i < vector_size(*scope); i++)
    {
        printf("%s\n", vector_get(*scope, i).identifier);
    }
    printf("----\n");
}