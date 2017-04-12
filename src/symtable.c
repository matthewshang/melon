#include "symtable.h"

#include <string.h>
#include <stdio.h>

symtable_t *symtable_new()
{
    symtable_t *table = (symtable_t*)calloc(1, sizeof(symtable_t));
    vector_init(table->stack);
    table->top = 0;
    string_r *first_scope = (string_r*)calloc(1, sizeof(string_r));
    vector_init(*first_scope);
    vector_push(string_r*, table->stack, first_scope);
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

int symtable_index(symtable_t *table, const char *symbol)
{
    string_r *scope = vector_get(table->stack, table->top);
    for (size_t i = 0; i < vector_size(*scope); i++)
    {
        if (strcmp(symbol, vector_get(*scope, i)) == 0) return i;
    }
    return -1;
}

void symtable_add_local(symtable_t *table, const char *symbol)
{
    if (symtable_index(table, symbol) != -1)
    {
        printf("Symbol %s already defined\n", symbol);
        return;
    }

    string_r *scope = vector_get(table->stack, table->top);
    vector_push(const char*, *scope, symbol);
}

void symtable_enter_scope(symtable_t *table)
{
    table->top++;
    string_r *new_scope = (string_r*)calloc(1, sizeof(string_r));
    vector_init(*new_scope);
    vector_push(string_r*, table->stack, new_scope);
}

void symtable_exit_scope(symtable_t *table)
{
    vector_destroy(*vector_get(table->stack, table->top));
    free(vector_get(table->stack, table->top));
    table->top--;
}

void symtable_dump(symtable_t *table)
{
    printf("----Dumping symtable----\n");
    string_r *scope = vector_get(table->stack, table->top);
    for (size_t i = 0; i < vector_size(*scope); i++)
    {
        printf("%s\n", vector_get(*scope, i));
    }
    printf("----\n");
}