#include "semantic.h"

#include "astwalker.h"
#include "symtable.h"

static bool whitespace_char(char c)
{
    return c == '\n' || c == '\t' || c == '\r';
}

static void print_error_line(const char *buffer, token_t token)
{
    const uint32_t max_length = 40;
    uint32_t start = token.offset;
    uint32_t end = token.offset;
    while (start >= 0 && token.offset - start < max_length
        && !whitespace_char(buffer[start]))
    {
        if (start == 0) break;
        start--;
    }
    if (whitespace_char(buffer[start])) start++;
    while (end < strlen(buffer) && end - token.offset < max_length
        && !whitespace_char(buffer[end]))
    {
        end++;
    }

    char linestr[128];
    sprintf(linestr, "line %d: ", token.line);
    printf(linestr);
    while (start < end)
    {
        putchar(buffer[start++]);
    }
    printf("\n");
    for (uint32_t i = 0; i < strlen(linestr) + token.col; i++)
    {
        putchar(' ');
    }
    printf("^\n");
}

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
        print_error_line(((lexer_t*)self->data2)->source.buffer, node->base.token);
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
        print_error_line(((lexer_t*)self->data2)->source.buffer, node->base.token);
        printf("[Error] Function %s is already defined\n", node->identifier);
        self->nerrors++;
        return;
    }
    symtable_add_local(symtable, node->identifier);
}

bool sema_build_global_symtables(node_t *ast, lexer_t *lexer)
{
    astwalker_t walker = {
        .nerrors = 0,
        .depth = 0,
        .data = NULL,
        .data2 = (void*)lexer,

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

bool semantic_process(node_t *ast, lexer_t *lexer)
{
    if (!sema_build_global_symtables(ast, lexer))
        return false;
}