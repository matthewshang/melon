#ifndef __CODEGEN__
#define __CODEGEN__

#include <stdint.h>

#include "ast.h"
#include "symtable.h"
#include "value.h"
#include "vector.h"

typedef struct
{
    byte_r *code;
    string_r locals;
    value_r *constants;
    function_t *func;

} codegen_t;

codegen_t codegen_create(function_t *f);
void codegen_run(codegen_t *gen, node_t *ast);

#endif