#include "codegen.h"

#include <string.h>
#include <stdbool.h>

#include "astwalker.h"
#include "opcodes.h"

#define CODE_R ((codegen_t*)self->data)->code
#define LOCALS ((codegen_t*)self->data)->locals

#define AS_GEN(self) ((codegen_t*)self->data)

static void emit_byte(byte_r *code, uint8_t b)
{
    vector_push(uint8_t, *code, b);
}

static void emit_bytes(byte_r *code, uint8_t b1, uint8_t b2)
{
    vector_push(uint8_t, *code, b1);
    vector_push(uint8_t, *code, b2);

}

static void gen_node_prog(astwalker_t *self, node_prog_t *node)
{
    for (int i = 0; i < vector_size(*node->stmts); i++)
    {
        walk_ast(self, vector_get(*node->stmts, i));
    }
}

static void gen_node_if(astwalker_t *self, node_if_t *node)
{
    walk_ast(self, node->cond);
    emit_bytes(&CODE_R, (uint8_t)OP_JIF, 0);
    int idx = vector_size(CODE_R) - 1;
    walk_ast(self, node->then);
    
    if (node->els)
    {
        // + 2 to skip (jmp n) instruction
        int jmp = vector_size(CODE_R) - idx + 2;
        vector_set(CODE_R, idx, (uint8_t)jmp);

        emit_bytes(&CODE_R, (uint8_t)OP_JMP, 0);
        int idx2 = vector_size(CODE_R) - 1;
        walk_ast(self, node->els);
        jmp = vector_size(CODE_R) - idx2;
        vector_set(CODE_R, idx2, (uint8_t)jmp);
    }
    else
    {
        int jmp = vector_size(CODE_R) - idx;
        vector_set(CODE_R, idx, (uint8_t)jmp);
    }
}

static void gen_node_loop(astwalker_t *self, node_loop_t *node)
{
    int loop_idx = vector_size(CODE_R) - 1;
    walk_ast(self, node->cond);

    emit_bytes(&CODE_R, (uint8_t)OP_JIF, 0);
    int jif_idx = vector_size(CODE_R) - 1;
    walk_ast(self, node->body);
    int loop_jmp = vector_size(CODE_R) - loop_idx;

    emit_bytes(&CODE_R, (uint8_t)OP_LOOP, (uint8_t)loop_jmp);

    int jif_jmp = vector_size(CODE_R) - jif_idx;
    vector_set(CODE_R, jif_idx, (uint8_t)jif_jmp);
}

static bool identifier_is_unique(string_r *vars, const char *identifier)
{
    for (int i = 0; i < vector_size(*vars); i++)
    {
        if (strcmp(vector_get(*vars, i), identifier) == 0) return false;
    }

    return true;
}

static int identifier_index(string_r *vars, const char *identifier)
{
    for (int i = 0; i < vector_size(*vars); i++)
    {
        if (strcmp(vector_get(*vars, i), identifier) == 0) return i;
    }

    return -1;
}

static void gen_node_var_decl(astwalker_t *self, node_var_decl_t *node)
{
    if (!identifier_is_unique(&LOCALS, node->ident))
    {
        printf("Identifier %s already defined\n", node->ident);
        return;
    }
    vector_push(const char*, LOCALS, node->ident);
    if (node->init) 
    {
        walk_ast(self, node->init);
        emit_bytes(&CODE_R, (uint8_t)OP_STORE, (uint8_t)vector_size(LOCALS) - 1);
    }
}

static void gen_node_binary(astwalker_t *self, node_binary_t *node)
{
    if (node->op.type == TOK_EQ)
    {
        walk_ast(self, node->right);
        node->left->is_assign = true;
        walk_ast(self, node->left);
        return;
    }
    walk_ast(self, node->left);
    walk_ast(self, node->right);
    emit_byte(&CODE_R, (uint8_t)token_to_binary_op(node->op));
}

static void gen_node_unary(astwalker_t *self, node_unary_t *node)
{
    walk_ast(self, node->right);
    emit_byte(&CODE_R, (uint8_t)token_to_unary_op(node->op));
}

static void gen_node_call(astwalker_t *self, node_call_t *node)
{
    for (int i = 0; i < vector_size(*node->args); i++)
    {
        walk_ast(self, vector_get(*node->args, i));
    }
    emit_byte(&CODE_R, (uint8_t)OP_CALL);
}

static void gen_node_var(astwalker_t *self, node_var_t *node)
{
    int index = identifier_index(&LOCALS, node->identifier);
    if (index == -1)
    {
        printf("Unknown identifier %s\n", node->identifier);
        return;
    }

    if (node->base.is_assign)
        emit_bytes(&CODE_R, (uint8_t)OP_STORE, (uint8_t)index);
    else
        emit_bytes(&CODE_R, (uint8_t)OP_LOAD, (uint8_t)index);
}

static void gen_node_literal(astwalker_t *self, node_literal_t *node)
{
    switch (node->type)
    {
    case LITERAL_BOOL:
    case LITERAL_INT:
    {
        emit_bytes(&CODE_R, OP_LOADI, (uint8_t)node->u.i);
        break;
    }
    case LITERAL_STR:
    {
        emit_bytes(&CODE_R, OP_LOADK, (uint8_t)vector_size(AS_GEN(self)->constants));
        value_t str = FROM_CSTR(node->u.s);
        vector_push(value_t, AS_GEN(self)->constants, str);
        break;
    }
    default: break;
    }
}

codegen_t codegen_create()
{
    codegen_t gen;
    vector_init(gen.code);
    vector_init(gen.locals);
    vector_init(gen.constants);
    return gen;
}

void codegen_destroy(codegen_t *gen)
{
    vector_destroy(gen->code);
    vector_destroy(gen->locals);
    vector_destroy(gen->constants);
}

void codegen_run(codegen_t *gen, node_t *ast)
{
    astwalker_t walker = {
        .depth = 0,
        .data = (void*)gen,

        .visit_prog = gen_node_prog,
        .visit_if = gen_node_if,
        .visit_loop = gen_node_loop,

        .visit_var_decl = gen_node_var_decl,

        .visit_binary = gen_node_binary,
        .visit_unary = gen_node_unary,
        .visit_call = gen_node_call,
        .visit_var = gen_node_var,
        .visit_literal = gen_node_literal
    };

    walk_ast(&walker, ast);
    emit_byte(&gen->code, (uint8_t)OP_HALT);
}