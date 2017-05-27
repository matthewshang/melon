#include "codegen.h"

#include <string.h>
#include <stdbool.h>

#include "astwalker.h"
#include "core.h"
#include "hash.h"
#include "opcodes.h"

#define CODE ((codegen_t*)self->data)->code
#define LOCALS ((codegen_t*)self->data)->locals
#define CONSTANTS ((codegen_t*)self->data)->constants
#define SYMTABLE ((codegen_t*)self->data)->symtable

#define GET_CONTEXT vector_peek(((codegen_t*)self->data)->decls)
//#define PUSH_CONTEXT(x) vector_push(value_t, ((codegen_t*)self->data)->decls, x)
#define PUSH_CONTEXT(x) push_context((codegen_t*)self->data, x)
//#define POP_CONTEXT vector_pop(((codegen_t*)self->data)->decls)
#define POP_CONTEXT pop_context((codegen_t*)self->data)

#define AS_GEN(self) ((codegen_t*)self->data)

#define MAX_LITERAL_INT 256

static void push_context(codegen_t *gen, value_t context)
{
    vector_push(value_t, gen->decls, context);
    if (IS_CLOSURE(context))
    {
        function_t *f = AS_CLOSURE(context)->f;
        gen->code = &f->bytecode;
        gen->constants = &f->constpool;
    }
}

static void pop_context(codegen_t *gen)
{
    vector_pop(gen->decls);
    value_t context = vector_peek(gen->decls);
    if (IS_CLOSURE(context))
    {
        function_t *f = AS_CLOSURE(context)->f;
        gen->code = &f->bytecode;
        gen->constants = &f->constpool;
    }
}

static void emit_byte(byte_r *code, uint8_t b)
{
    vector_push(uint8_t, *code, b);
}

static void emit_bytes(byte_r *code, uint8_t b1, uint8_t b2)
{
    vector_push(uint8_t, *code, b1);
    vector_push(uint8_t, *code, b2);
}

static void emit_loadstore(byte_r *code, location_e loc, uint8_t idx, bool store)
{
    if (loc == LOC_GLOBAL)
    {
        emit_bytes(code, store ? OP_STOREG : OP_LOADG, idx);
    }
    else if (loc == LOC_LOCAL)
    {
        emit_bytes(code, store ? OP_STOREL : OP_LOADL, idx);
    }
    else if (loc == LOC_UPVALUE)
    {
        emit_bytes(code, store ? OP_STOREU : OP_LOADU, idx);
    }
    else if (loc == LOC_CLASS)
    {
        emit_byte(code, store ? OP_STOREF : OP_LOADF);
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

static void store_decl(astwalker_t *self, value_t decl, node_func_decl_t *node)
{
    value_t context = GET_CONTEXT;
    bool env_local = IS_CLOSURE(context);
    bool env_class = IS_CLASS(context);

    if (env_local)
    {
        function_t *contextf = AS_CLOSURE(context)->f;
        vector_push(value_t, contextf->constpool, decl);
        emit_bytes(&contextf->bytecode, (uint8_t)OP_LOADK, vector_size(contextf->constpool) - 1);

        if (IS_CLOSURE(decl))
        {
            function_t *f = AS_CLOSURE(decl)->f;

            vector_t(ast_upvalue_t) *upvalues = node->upvalues;
            f->nupvalues = vector_size(*upvalues);
            emit_byte(CODE, (uint8_t)OP_CLOSURE);

            uint8_t upindex = 0;
            for (uint8_t i = 0; i < f->nupvalues; i++)
            {
                ast_upvalue_t upvalue = vector_get(*upvalues, i);
                emit_bytes(&contextf->bytecode, (uint8_t)OP_NEWUP, (uint8_t)upvalue.is_direct);
                emit_byte(&contextf->bytecode, upvalue.is_direct ? upvalue.idx : upindex++);
            }
        }
    }
}

static uint8_t cpool_add_constant(value_r *cpool, value_t v)
{
    if (vector_size(*cpool) > 255)
    {
        printf("error: maximum amount of constants\n");
        return 255;
    }

    for (int i = 0; i < vector_size(*cpool); i++)
    {
        value_t val = vector_get(*cpool, i);
        if (value_equals(val, v)) return i;
    }

    vector_push(value_t, *cpool, v);
    return vector_size(*cpool) - 1;
}

static void gen_node_var_decl(astwalker_t *self, node_var_decl_t *node)
{
    value_t context = GET_CONTEXT;
    bool env_local = IS_CLOSURE(context);
    bool env_class = IS_CLASS(context);

    if (env_local)
    {
        if (node->init)
        {
            walk_ast(self, node->init);
            emit_loadstore(CODE, node->loc, node->idx, true);
        }
        else
        {
            emit_bytes(CODE, (uint8_t)OP_LOADI, 0);
        }
    }
    else if (env_class)
    {
        class_t *c = AS_CLASS(context);
        class_bind(c, FROM_CSTR(strdup(node->ident)), FROM_INT(node->idx));

        if (node->init)
        {
            closure_t *initf = class_lookup_closure(c, FROM_CSTR("$init"));
            if (!initf) return;

            PUSH_CONTEXT(FROM_CLOSURE(initf));
            walk_ast(self, node->init);
            POP_CONTEXT;

            emit_bytes(&initf->f->bytecode, (uint8_t)OP_LOADL, 0);
            emit_bytes(&initf->f->bytecode, (uint8_t)OP_LOADI, node->idx);
            emit_loadstore(&initf->f->bytecode, LOC_CLASS, node->idx, true);
        }
    }
}

static void gen_node_func_decl(astwalker_t *self, node_func_decl_t *node)
{
    function_t *f = function_new(strdup(node->identifier));
    closure_t *cl = closure_new(f);

    PUSH_CONTEXT(FROM_CLOSURE(cl));

    walk_ast(self, node->body);
    if (vector_get(*CODE, vector_size(*CODE) - 1) != OP_RETURN)
        emit_byte(CODE, (uint8_t)OP_RET0);

    POP_CONTEXT;

    store_decl(self, FROM_CLOSURE(cl), node);
}

static void gen_node_class_decl(struct astwalker *self, node_class_decl_t *node)
{
    class_t *c = class_new(strdup(node->identifier), node->num_instvars);
    closure_t *init = closure_new(function_new(strdup("$init")));
    class_bind(c, FROM_CSTR(strdup("$init")), FROM_CLOSURE(init));

    PUSH_CONTEXT(FROM_CLASS(c));

    for (size_t i = 0; i < vector_size(*node->decls); i++)
    {
        walk_ast(self, vector_get(*node->decls, i));
    }

    POP_CONTEXT;

    emit_bytes(&init->f->bytecode, (uint8_t)OP_LOADL, 0);
    emit_byte(&init->f->bytecode, (uint8_t)OP_RETURN);

    store_decl(self, FROM_CLASS(c), NULL);


    emit_loadstore(CODE, LOC_GLOBAL, node->idx, true);
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
    //uint8_t nargs = 0;
    //if (node->type == POST_CALL && node->args)
    //{
    //    for (size_t i = 0; i < vector_size(*node->args); i++)
    //    {
    //        walk_ast(self, vector_get(*node->args, i));
    //    }
    //    nargs = vector_size(*node->args);
    //}

    //walk_ast(self, node->target);

    //if (node->type == POST_CALL)
    //{
    //    emit_bytes(CODE, (uint8_t)OP_CALL, nargs);
    //}
    //else if (node->type == POST_ACCESS)
    //{
    //    node_var_t *expr = (node_var_t*)node->expr;
    //    emit_bytes(CODE, (uint8_t)OP_LOADK, 
    //        cpool_add_constant(CONSTANTS, FROM_CSTR(strdup(expr->identifier))));
    //    emit_byte(CODE, node->base.is_assign ? OP_STOREF : OP_LOADF);
    //}
}

static void gen_node_var(astwalker_t *self, node_var_t *node)
{
    if (node->location == LOC_CLASS)
    {
        emit_bytes(CODE, OP_LOADL, 0);
        emit_bytes(CODE, OP_LOADI, node->idx);
    }
    emit_loadstore(CODE, node->location, node->idx, node->base.is_assign);
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
    function_t *context = AS_CLOSURE(GET_CONTEXT)->f;
    value_r *constpool = &context->constpool;
    byte_r *code = &context->bytecode;

    int index = constant_exists(constpool, node);
    if (index != -1)
    {
        emit_bytes(code, OP_LOADK, (uint8_t)index);
        return;
    }

    switch (node->type)
    {
    case LITERAL_BOOL:
    {
        emit_bytes(code, OP_LOADK, (uint8_t)vector_size(*constpool));
        vector_push(value_t, *constpool, FROM_BOOL(node->u.i));
        break;
    }
    case LITERAL_INT:
    {
        if (node->u.i < MAX_LITERAL_INT)
        {
            emit_bytes(code, OP_LOADI, (uint8_t)node->u.i);
        }
        else
        {
            emit_bytes(code, OP_LOADK, (uint8_t)vector_size(*constpool));
            vector_push(value_t, *constpool, FROM_INT(node->u.i));
        }
        break;
    }
    case LITERAL_FLT:
    {
        emit_bytes(code, OP_LOADK, (uint8_t)vector_size(*constpool));
        vector_push(value_t, *constpool, FROM_FLOAT(node->u.d));
        break;
    }
    case LITERAL_STR:
    {
        emit_bytes(code, OP_LOADK, (uint8_t)vector_size(*constpool));
        value_t str = FROM_CSTR(strdup(node->u.s));
        vector_push(value_t, *constpool, str);
        break;
    }
    default: break;
    }
}

codegen_t codegen_create(function_t *f)
{
    codegen_t gen;

    gen.main_cl = closure_new(f);
    gen.code = &f->bytecode;
    gen.constants = &f->constpool;
    vector_init(gen.decls);
    vector_push(value_t, gen.decls, FROM_CLOSURE(gen.main_cl));
    return gen;
}

void codegen_destroy(codegen_t *gen)
{
    vector_destroy(gen->decls);
    free(gen->main_cl);
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