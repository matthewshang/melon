#ifndef __SYMTABLE__
#define __SYMTABLE__

#include <stdbool.h>

#include "ast.h"
#include "vector.h"

typedef struct
{
    bool is_global;
    uint8_t idx;
    uint8_t level;
} decl_info_t;

typedef struct
{
    const char *identifier;
    decl_info_t decl;
} symtable_entry_t;

typedef vector_t(symtable_entry_t) symtable_entry_r;

typedef struct symtable_t
{
    // linear search for now
    vector_t(symtable_entry_r*) stack;
    uint32_t top;
} symtable_t;

symtable_t *symtable_new();
void symtable_free(symtable_t *table);

bool symtable_lookup(symtable_t *table, const char *symbol, decl_info_t *ret);
uint8_t symtable_add_local(symtable_t *table, const char *symbol);
void symtable_modify_decl(symtable_t *table, const char *symbol, uint8_t idx);
uint8_t symtable_nvars(symtable_t *table);
void symtable_enter_scope(symtable_t *table);
uint32_t symtable_exit_scope(symtable_t *table);

bool symtable_is_global(symtable_t *table);

void symtable_dump(symtable_t *table);

#endif