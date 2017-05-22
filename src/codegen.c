#include "codegen.h"

#include <string.h>
#include <stdbool.h>

#include "astwalker.h"
#include "core.h"
#include "opcodes.h"

#define CODE ((codegen_t*)self->data)->code
#define LOCALS ((codegen_t*)self->data)->locals
#define CONSTANTS ((codegen_t*)self->data)->constants
#define SYMTABLE ((codegen_t*)self->data)->symtable

#define AS_GEN(self) ((codegen_t*)self->data)

#define MAX_LITERAL_INT 256

static void emit_byte(byte_r *code, uint8_t b)
{
    vector_push(uint8_t, *code, b);
}

static void emit_bytes(byte_r *code, uint8_t b1, uint8_t b2)
{
    vector_push(uint8_t, *code, b1);
    vector_push(uint8_t, *code, b2);
}

static void emit_var(byte_r *code, location_e loc, uint8_t idx, bool store)
{
    if (loc == LOC_GLOBAL)
    {
        emit_bytes(code, store ? OP_STOREG : OP_LOADG, idx);
    }
    else if (loc == LOC_LOCAL)
    {
        emit_bytes(code, store ? OP_STORE : OP_LOAD, idx);
    }
    else if (loc == LOC_UPVALUE)
    {
        emit_bytes(code, store ? OP_STOREU : OP_LOADU, idx);
    }
}

static void gen_node_block(astwalker_t *self, node_block_t *node)
{
    for (int i = 0; i < vector_size(*node->stmts); i++)
    {
        walk_ast(self, vector_get(*node->stmts, i));
    }
}

static void gen_node_if(astwalker_t *self, node_if_t *node)
{
    walk_ast(self, node->cond);
    emit_bytes(CODE, (uint8_t)OP_JIF, 0);
    int idx = vector_size(*CODE) - 1;
    walk_ast(self, node->then);
    
    if (node->els)
    {
        // + 2 to skip (jmp n) instruction
        int jmp = vector_size(*CODE) - idx + 2;
        vector_set(*CODE, idx, (uint8_t)jmp);

        emit_bytes(CODE, (uint8_t)OP_JMP, 0);
        int idx2 = vector_size(*CODE) - 1;
        walk_ast(self, node->els);
        jmp = vector_size(*CODE) - idx2;
        vector_set(*CODE, idx2, (uint8_t)jmp);
    }
    else
    {
        int jmp = vector_size(*CODE) - idx;
        vector_set(*CODE, idx, (uint8_t)jmp);
    }
}

static void gen_node_loop(astwalker_t *self, node_loop_t *node)
{
    int loop_idx = vector_size(*CODE) - 1;
    walk_ast(self, node->cond);

    emit_bytes(CODE, (uint8_t)OP_JIF, 0);
    int jif_idx = vector_size(*CODE) - 1;
    walk_ast(self, node->body);
    int loop_jmp = vector_size(*CODE) - loop_idx;

    emit_bytes(CODE, (uint8_t)OP_LOOP, (uint8_t)loop_jmp);

    int jif_jmp = vector_size(*CODE) - jif_idx;
    vector_set(*CODE, jif_idx, (uint8_t)jif_jmp);
}

static void gen_node_return(astwalker_t *self, node_return_t *node)
{
    walk_ast(self, node->expr);
    emit_byte(CODE, (uint8_t)OP_RETURN);
}

static void gen_node_var_decl(astwalker_t *self, node_var_decl_t *node)
{
    if (node->init)
    {
        //printf("var_decl: %d %d\n", node->loc, node->idx);
        walk_ast(self, node->init);
        emit_var(CODE, node->loc, node->idx, true);
    }
}

static void gen_node_func_decl(astwalker_t *self, node_func_decl_t *node)
{
    //printf("func_decl: %d %d\n", node->loc, node->idx);

    function_t *f = function_new(strdup(node->identifier));
    codegen_t *gen = (codegen_t*)self->data;
    function_t *parent = gen->func;
    gen->func = f;
    gen->code = &f->bytecode;
    gen->constants = &f->constpool;

    walk_ast(self, node->body);
    if (vector_get(*gen->code, vector_size(*gen->code) - 1) != OP_RETURN)
        emit_byte(CODE, (uint8_t)OP_RET0);

    gen->func = parent;
    gen->code = &parent->bytecode;
    gen->constants = &parent->constpool;

    vector_push(value_t, parent->constpool, FROM_CLOSURE(closure_new(f)));

    emit_bytes(CODE, (uint8_t)OP_LOADK, vector_size(parent->constpool) - 1);

    vector_t(ast_upvalue_t) *upvalues = node->upvalues;
    f->nupvalues = vector_size(*upvalues);
    emit_byte(CODE, (uint8_t)OP_CLOSURE);

    uint8_t upindex = 0;
    for (uint8_t i = 0; i < f->nupvalues; i++)
    {
        ast_upvalue_t upvalue = vector_get(*upvalues, i);
        emit_bytes(CODE, (uint8_t)OP_NEWUP, (uint8_t)upvalue.is_direct);
        emit_byte(CODE, upvalue.is_direct ? upvalue.idx : upindex++);
    }

    if (!node->base.is_assign)
        emit_var(CODE, node->loc, node->idx, true);
}

static void gen_node_class_decl(struct astwalker *self, node_class_decl_t *node)
{
    for (size_t i = 0; i < vector_size(*node->decls); i++)
    {
        walk_ast(self, vector_get(*node->decls, i));
    }
}

static void gen_node_binary(astwalker_t *self, node_binary_t *node)
{
    if (node->op.type == TOK_EQ)
    {
        if (node->right->type == NODE_FUNC_DECL)
            node->right->is_assign = true;
        walk_ast(self, node->right);
        node->left->is_assign = true;
        walk_ast(self, node->left);
        return;
    }
    walk_ast(self, node->left);
    walk_ast(self, node->right);
    emit_byte(CODE, (uint8_t)token_to_binary_op(node->op));
}

static void gen_node_unary(astwalker_t *self, node_unary_t *node)
{
    walk_ast(self, node->right);
    emit_byte(CODE, (uint8_t)token_to_unary_op(node->op));
}

static void gen_node_postfix(astwalker_t *self, node_postfix_t *node)
{
    if (node->args)
    {
        for (size_t i = 0; i < vector_size(*node->args); i++)
        {
            walk_ast(self, vector_get(*node->args, i));
        }
        walk_ast(self, node->target);
        emit_bytes(CODE, (uint8_t)OP_CALL, vector_size(*node->args));
    }
    else
    {
        walk_ast(self, node->target);
        emit_bytes(CODE, (uint8_t)OP_CALL, 0);
    }
}

static void gen_node_var(astwalker_t *self, node_var_t *node)
{
    emit_var(CODE, node->location, node->idx, node->base.is_assign);
}

static int constant_exists(value_r *constants, node_literal_t *node)
{
    value_e type = node->type;
    for (int i = 0; i < vector_size(*constants); i++)
    {
        value_t val = vector_get(*constants, i);
        if (val.type == type)
        {
            if (val.type == VAL_STR)
                if (strcmp(node->u.s, val.s) == 0) return i;
            if (val.type == VAL_BOOL || val.type == VAL_INT)
                if (node->u.i == val.i) return i;
            if (val.type == VAL_FLOAT)
                if (node->u.d == val.d) return i;
        }
    }

    return -1;
}

static void gen_node_literal(astwalker_t *self, node_literal_t *node)
{
    int index = constant_exists(CONSTANTS, node);
    if (index != -1)
    {
        emit_bytes(CODE, OP_LOADK, (uint8_t)index);
        return;
    }

    switch (node->type)
    {
    case LITERAL_BOOL:
    {
        emit_bytes(CODE, OP_LOADK, (uint8_t)vector_size(*CONSTANTS));
        vector_push(value_t, *CONSTANTS, FROM_BOOL(node->u.i));
        break;
    }
    case LITERAL_INT:
    {
        if (node->u.i < MAX_LITERAL_INT)
        {
            emit_bytes(CODE, OP_LOADI, (uint8_t)node->u.i);
        }
        else
        {
            emit_bytes(CODE, OP_LOADK, (uint8_t)vector_size(*CONSTANTS));
            vector_push(value_t, *CONSTANTS, FROM_INT(node->u.i));
        }
        break;
    }
    case LITERAL_FLT:
    {
        emit_bytes(CODE, OP_LOADK, (uint8_t)vector_size(*CONSTANTS));
        vector_push(value_t, *CONSTANTS, FROM_FLOAT(node->u.d));
        break;
    }
    case LITERAL_STR:
    {
        emit_bytes(CODE, OP_LOADK, (uint8_t)vector_size(*CONSTANTS));
        value_t str = FROM_CSTR(strdup(node->u.s));
        vector_push(value_t, *CONSTANTS, str);
        break;
    }
    default: break;
    }
}

codegen_t codegen_create(function_t *f)
{
    codegen_t gen;

    gen.func = f;
    gen.code = &gen.func->bytecode;
    gen.constants = &gen.func->constpool;
    return gen;
}

void codegen_run(codegen_t *gen, node_t *ast)
{
    astwalker_t walker = {
        .depth = 0,
        .data = (void*)gen,

        .visit_block = gen_node_block,
        .visit_if = gen_node_if,
        .visit_loop = gen_node_loop,
        .visit_return = gen_node_return,

        .visit_var_decl = gen_node_var_decl,
        .visit_func_decl = gen_node_func_decl,
        .visit_class_decl = gen_node_class_decl,

        .visit_binary = gen_node_binary,
        .visit_unary = gen_node_unary,
        .visit_postfix = gen_node_postfix,
        .visit_var = gen_node_var,
        .visit_literal = gen_node_literal
    };

    walk_ast(&walker, ast);
    emit_byte(gen->code, (uint8_t)OP_HALT);
}