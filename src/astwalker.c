#include "astwalker.h"

#define VISIT(type)    if (!self->visit_##type) return;                    \
                       self->visit_##type(self, (node_##type##_t *) node); \
                       break;

void walk_ast(astwalker_t *self, node_t *node)
{
    switch (node->type)
    {
    case NODE_BLOCK: VISIT(block);
    case NODE_IF: VISIT(if);
    case NODE_LOOP: VISIT(loop);
    case NODE_RETURN: VISIT(return);

    case NODE_VAR_DECL: VISIT(var_decl);
    case NODE_FUNC_DECL: VISIT(func_decl);

    case NODE_BINARY: VISIT(binary);
    case NODE_UNARY: VISIT(unary);
    case NODE_CALL: VISIT(call);
    case NODE_POSTFIX: VISIT(postfix);
    case NODE_VAR: VISIT(var);
    case NODE_LITERAL: VISIT(literal);
    }
}