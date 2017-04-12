#ifndef __SYMTABLE__
#define __SYMTABLE__

#include "vector.h"

typedef struct
{
    // linear search for now
    vector_t(string_r*) stack;
    uint32_t top;
} symtable_t;

symtable_t *symtable_new();
void symtable_free(symtable_t *table);

int symtable_index(symtable_t *table, const char *symbol);
void symtable_add_local(symtable_t *table, const char *symbol);
void symtable_enter_scope(symtable_t *table);
void symtable_exit_scope(symtable_t *table);

void symtable_dump(symtable_t *table);

#endif