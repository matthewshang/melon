#ifndef __SEMANTIC__
#define __SEMANTIC__

#include <stdbool.h>

#include "ast.h"
#include "lexer.h"

bool semantic_process(node_t *ast, lexer_t *lexer);

#endif