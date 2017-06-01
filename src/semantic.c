#include "semantic.h"

#include <stdarg.h>

#include "astwalker.h"
#include "core.h"
#include "symtable.h"

#define MAX_LOCALS 255

#define GET_CONTEXT vector_peek(((semantic_t*)self->data)->context_stack)
#define PUSH_CONTEXT(x) vector_push(node_t*, ((semantic_t*)self->data)->context_stack, x)
#define POP_CONTEXT vector_pop(((semantic_t*)self->data)->context_stack)

typedef struct
{
    node_r context_stack;

} semantic_t;

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
    sprintf(linestr, "        ");
    printf(linestr);
    while (start < end)
    {
        putchar(buffer[start++]);
    }
    printf("\n");

    uint8_t ntabs = 0;
    int i = token.offset;
    while (i >= 0 && (int)token.offset - i <= token.col)
    {
        if (buffer[i] == '\t') ntabs++;
        if (buffer[i] == '\n') break;
        i--;
    }

    for (uint32_t i = 0; i < strlen(linestr) + token.col - ntabs; i++)
    {
        putchar(' ');
    }
    printf("^\n");
}

static void report_error(const char *msg, ...)
{
    printf("[Error::Semantic] ");
    va_list args;

    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
    printf("\n");
}

static void semantic_error(astwalker_t *self, token_t token, const char *msg, ...)
{
    printf("line %d: error: ", token.line);
    va_list args;

    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);

    print_error_line(((lexer_t*)self->data2)->source.buffer, token);

    self->nerrors++;
}

static void visit_block_global(struct astwalker *self, node_block_t *node)
{
    for (size_t i = 0; i < vector_size(*node->stmts); i++)
    {
        walk_ast(self, vector_get(*node->stmts, i));
    }

    node->symtable = (symtable_t*)self->data;
}

static void visit_var_decl_global(struct astwalker *self, node_var_decl_t *node)
{
    symtable_t *symtable = (symtable_t*)self->data;
    if (symtable_lookup(symtable, node->ident, NULL))
    {
        semantic_error(self, node->base.token, "Variable %s is already defined\n", node->ident);
        return;
    }
    node->idx = symtable_add_local(symtable, node->ident);
    node->loc = LOC_GLOBAL;
}

static void visit_func_decl_global(struct astwalker *self, node_func_decl_t *node)
{
    symtable_t *symtable = (symtable_t*)self->data;
    if (symtable_lookup(symtable, node->identifier, NULL))
    {
        semantic_error(self, node->base.token, "Function %s is already defined\n", node->identifier);
        return;
    }
    node->idx = symtable_add_local(symtable, node->identifier);
    node->loc = LOC_GLOBAL;
}

static void visit_class_decl_global(struct astwalker *self, node_class_decl_t *node)
{
    symtable_t *symtable = (symtable_t*)self->data;
    if (symtable_lookup(symtable, node->identifier, NULL))
    {
        semantic_error(self, node->base.token, "Class %s is already defined\n", node->identifier);
        return;
    }
    node->idx = symtable_add_local(symtable, node->identifier);
    node->loc = LOC_GLOBAL;

    node->symtable = symtable_new();
    if (node->decls)
    {
        for (size_t i = 0; i < vector_size(*node->decls); i++)
        {
            node_t *decl = vector_get(*node->decls, i);
            const char *ident = NULL;
            if (decl->type == NODE_VAR_DECL)
            {
                ident = ((node_var_decl_t*)decl)->ident;
            }
            else
            {
                semantic_error(self, decl->token, "Class declarations must be a variable or function\n");
                return;
            }
            symtable_add_local(node->symtable, ident);
        }
    }
}

static bool sema_build_global_symtables(node_t *ast, lexer_t *lexer)
{
    symtable_t *globals = symtable_new();
    core_register_semantic(globals);

    astwalker_t walker = {
        .nerrors = 0,
        .depth = 0,
        .data = globals,
        .data2 = (void*)lexer,

        .visit_block = visit_block_global,
        .visit_if = NULL,
        .visit_loop = NULL,
        .visit_return = NULL,

        .visit_var_decl = visit_var_decl_global,
        .visit_func_decl = visit_func_decl_global,
        .visit_class_decl = visit_class_decl_global,

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

static symtable_t *node_get_symtable(node_t *node)
{
    if (node->type == NODE_BLOCK)
        return ((node_block_t*)node)->symtable;
    else if (node->type == NODE_FUNC_DECL)
        return ((node_func_decl_t*)node)->symtable;
    else if (node->type == NODE_CLASS_DECL)
        return ((node_class_decl_t*)node)->symtable;

    report_error("Last declaration does not contain a symboltable\n");
    return NULL;
}

static bool context_is_root(node_t *node)
{
    if (node->type == NODE_BLOCK)
        return ((node_block_t*)node)->is_root;
    return false;
}

static void visit_block(struct astwalker *self, node_block_t *node)
{
    if (!node->is_root)
    {
        node->symtable = node_get_symtable(GET_CONTEXT);
        symtable_enter_scope(node->symtable);
    }
    else
    {
        //symtable_dump(node->symtable);
        PUSH_CONTEXT(node);
    }

    for (size_t i = 0; i < vector_size(*node->stmts); i++)
    {
        walk_ast(self, vector_get(*node->stmts, i));
    }

    if (!node->is_root)
    {
        symtable_exit_scope(node->symtable);
    }
}

static void visit_if(struct astwalker *self, node_if_t *node)
{
    walk_ast(self, node->cond);
    walk_ast(self, node->then);
    if (node->els) walk_ast(self, node->els);
}

static void visit_loop(struct astwalker *self, node_loop_t *node)
{
    walk_ast(self, node->cond);
    walk_ast(self, node->body);
}

static void visit_return(struct astwalker *self, node_return_t *node)
{
    walk_ast(self, node->expr);
}

static void visit_var_decl(struct astwalker *self, node_var_decl_t *node)
{
    node_t *context = GET_CONTEXT;
    symtable_t *env_symtable = node_get_symtable(context);
    bool env_class = context->type == NODE_CLASS_DECL;
    bool env_func = context->type == NODE_FUNC_DECL;

    if (node->init) walk_ast(self, node->init);

    if (env_func)
    {
        if (symtable_lookup(env_symtable, node->ident, NULL))
        {
            semantic_error(self, node->base.token, "Variable %s is already defined\n", node->ident);
            return;
        }

        node->idx = symtable_add_local(env_symtable, node->ident);
        node->loc = LOC_LOCAL;
    }
    else if (env_class)
    {
        node_class_decl_t *c = (node_class_decl_t*)context;
        if (node->init && node->init->type == NODE_FUNC_DECL && strcmp(node->ident, c->identifier) == 0) 
            c->constructor = node;

        node->idx = c->num_instvars++;
        node->loc = LOC_CLASS;
        printf("env_class: %s, %d\n", node->ident, node->idx);
    }
}

static void visit_func_decl(struct astwalker *self, node_func_decl_t *node)
{
    node->symtable = symtable_new();
    symtable_t *symtable = node->symtable;
    symtable_enter_scope(symtable);

    if (node->params)
    {
        for (size_t i = 0; i < vector_size(*node->params); i++)
        {
            node_var_t *param = vector_get(*node->params, i);
            symtable_add_local(symtable, param->identifier);
        }
    }

    PUSH_CONTEXT(node);

    for (size_t i = 0; i < vector_size(*node->body->stmts); i++)
    {
        walk_ast(self, vector_get(*node->body->stmts, i));
    }

    POP_CONTEXT;

    uint32_t nlocals = symtable_exit_scope(symtable);
    if (nlocals > MAX_LOCALS) 
        semantic_error(self, node->base.token, 
            "Maximum number of local variables reached in function %s\n", node->identifier);
}

static void visit_class_decl(struct astwalker *self, node_class_decl_t *node)
{
    PUSH_CONTEXT(node);
    for (size_t i = 0; i < vector_size(*node->decls); i++)
    {
        walk_ast(self, vector_get(*node->decls, i));
    }
    POP_CONTEXT;
}

static void visit_binary(struct astwalker *self, node_binary_t *node)
{
    walk_ast(self, node->left);
    walk_ast(self, node->right);
}

static void visit_unary(struct astwalker *self, node_unary_t *node)
{
    walk_ast(self, node->right);
}

static void visit_postfix(struct astwalker *self, node_postfix_t *node)
{
    for (size_t i = 0; i < vector_size(*node->exprs); i++)
    {
        postfix_expr_t *expr = vector_get(*node->exprs, i);
        if (expr->type == POST_CALL)
        {
            if (expr->args)
            {
                for (size_t i = 0; i < vector_size(*expr->args); i++)
                {
                    walk_ast(self, vector_get(*expr->args, i));
                }
            }
        }
    }


    walk_ast(self, node->target);
}

static uint8_t add_upvalue(node_func_decl_t *f, uint16_t distance, decl_info_t decl, const char *symbol)
{
    vector_t(ast_upvalue_t) *upvalues = f->upvalues;
    for (uint8_t i = 0; i < vector_size(*upvalues); i++)
    {
        ast_upvalue_t upvalue = vector_get(*upvalues, i);
        if (strcmp(symbol, upvalue.symbol) == 0) return i;
    }

    vector_push(ast_upvalue_t, *upvalues,
        ((ast_upvalue_t){.is_direct = distance == 2, .idx = decl.idx, .symbol = symbol }));
    return vector_size(*upvalues) - 1;
}

static void visit_var(struct astwalker *self, node_var_t *node)
{
    node_r context_stack = ((semantic_t*)self->data)->context_stack;

    uint16_t funcs_traversed = 0;
    uint16_t classes_traversed = 0;
    uint16_t len = vector_size(context_stack);
    for (int i = len - 1; i >= 0; i--)
    {
        node_t *context = vector_get(context_stack, i);
        symtable_t *symtable = node_get_symtable(context);

        bool context_is_global = context_is_root(context);
        bool context_is_func = context->type == NODE_FUNC_DECL;
        bool context_is_class = context->type == NODE_CLASS_DECL;

        if (context_is_func) funcs_traversed++;
        if (context_is_class) classes_traversed++;

        decl_info_t decl;
        if (!symtable_lookup(symtable, node->identifier, &decl))
            continue;

        if (context_is_global)
        {
            node->location = LOC_GLOBAL;
            node->idx = decl.idx;
            return;
        }

        if (context_is_func)
        {
            if (funcs_traversed > 1)
            {
                node->location = LOC_UPVALUE;
                
                uint8_t d = funcs_traversed;
                uint8_t j = vector_size(context_stack) - 2;
                node_func_decl_t *target = (node_func_decl_t*)vector_get(context_stack, len - 1);
                node->idx = add_upvalue(target, d--, decl, node->identifier);

                while (d > 1)
                {
                    node_func_decl_t *f = vector_get(context_stack, j);
                    add_upvalue(f, d--, decl, node->identifier);
                    --j;
                }
            }
            else
            {
                bool is_method = false;
                while (i >= 0)
                {
                    if (vector_get(context_stack, i)->type == NODE_CLASS_DECL)
                        is_method = true;
                    i--;
                }
                
                node->location = LOC_LOCAL;
                node->idx = decl.idx + is_method;
            }
            return;
        }

        if (context_is_class)
        {
            node->location = LOC_CLASS;
            node->idx = decl.idx;
            return;
        }

        report_error("Fatal internal error\n");
        self->nerrors++;
    }

    semantic_error(self, node->base.token, "Undeclared identifier %s\n", node->identifier);
}

static bool sema_build_local_symtables(node_t *ast, lexer_t *lexer)
{
    semantic_t sema;
    vector_init(sema.context_stack);

    astwalker_t walker = {
        .nerrors = 0,
        .depth = 0,
        .data = (void*)&sema,
        .data2 = (void*)lexer,

        .visit_block = visit_block,
        .visit_if = visit_if,
        .visit_loop = visit_loop,
        .visit_return = visit_return,

        .visit_var_decl = visit_var_decl,
        .visit_func_decl = visit_func_decl,
        .visit_class_decl = visit_class_decl,

        .visit_binary = visit_binary,
        .visit_unary = visit_unary,
        .visit_postfix = visit_postfix,
        .visit_var = visit_var,
        .visit_literal = NULL
    };

    ((node_block_t*)ast)->is_root = true;
    walk_ast(&walker, ast);

    return walker.nerrors == 0;
}

bool semantic_process(node_t *ast, lexer_t *lexer)
{
    if (!sema_build_global_symtables(ast, lexer))
        return false;

    return sema_build_local_symtables(ast, lexer);
}