#ifndef __VALUE__
#define __VALUE__

#include "vector.h"

typedef enum
{
    VAL_BOOL, VAL_INT, VAL_FLOAT, VAL_STR, VAL_FUNC
} value_e;

typedef struct function_s function_t;

typedef struct
{
    value_e type;
    union
    {
        int i;
        float f;
        const char *s;
        function_t *fn;
    };
} value_t;

typedef vector_t(value_t) value_r;

typedef struct function_s
{
    const char *identifier;
    value_r constpool;
    byte_r bytecode;
} function_t;

#define FROM_BOOL(x) (value_t){.type = VAL_BOOL, .i = x}
#define FROM_INT(x) (value_t){.type = VAL_INT, .i = x}
#define FROM_FLOAT(x) (value_t){.type = VAL_FLOAT, .f = x}
#define FROM_CSTR(x) (value_t){.type = VAL_STR, .s = x}
#define FROM_FUNC(x) (value_t){.type = VAL_FUNC, .fn = x}

#define AS_INT(x) (x).i
#define AS_BOOL(x) (x).i
#define AS_FUNC(x) (x).fn

void value_destroy(value_t val);

function_t *function_new(const char *identifier);
void function_free(function_t *func);
value_t function_cpool_get(function_t *func, int idx);
void function_cpool_dump(function_t *func);
void function_disassemble(function_t *func);


#endif