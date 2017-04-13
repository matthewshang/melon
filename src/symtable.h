#ifndef __SYMTABLE__
#define __SYMTABLE__

#include <stdbool.h>

#include "ast.h"
#include "vector.h"

typedef struct
{
    const char *identifier;
    node_t *node;
} decl_t;

typedef vector_t(decl_t) decl_r;

typedef struct symtable_t
{
    // linear search for now
    vector_t(decl_r*) stack;
    uint32_t top;
} symtable_t;

symtable_t *symtable_new();
void symtable_free(symtable_t *table);

node_t *symtable_lookup(symtable_t *table, const char *symbol);
int symtable_add_local(symtable_t *table, const char *symbol, node_t *node);
void symtable_enter_scope(symtable_t *table);
void symtable_exit_scope(symtable_t *table);

bool symtable_is_global(symtable_t *table);

void symtable_dump(symtable_t *table);

#endif