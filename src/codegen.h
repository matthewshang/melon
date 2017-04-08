#ifndef __CODEGEN__
#define __CODEGEN__

#include <stdint.h>

#include "ast.h"
#include "value.h"
#include "vector.h"

typedef vector_t(uint8_t) byte_r;
typedef vector_t(const char*) string_r;

typedef struct
{
    byte_r code;
    string_r locals;
    value_r constants;
} codegen_t;

codegen_t codegen_create();
void codegen_destroy(codegen_t *gen);
void codegen_run(codegen_t *gen, node_t *ast);

#endif