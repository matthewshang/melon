#ifndef __DEBUG__
#define __DEBUG__

#include "opcodes.h"
#include "codegen.h"

const char *op_to_str(opcode op);
void disassemble_code(byte_r *code);

#endif