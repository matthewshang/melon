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
        double d;
        const char *s;
        function_t *fn;
    };
} value_t;

typedef vector_t(value_t) value_r;

typedef enum
{
    FUNC_MELON, FUNC_NATIVE
} function_e;

typedef void(*melon_c_func)(value_t *args, uint8_t nargs);

typedef struct function_s
{
    function_e type;
    union
    {
        struct
        {
            const char *identifier;
            value_r constpool;
            byte_r bytecode;
        };

        melon_c_func cfunc;
    };
} function_t;

#define FROM_BOOL(x) (value_t){.type = VAL_BOOL, .i = x}
#define FROM_INT(x) (value_t){.type = VAL_INT, .i = x}
#define FROM_FLOAT(x) (value_t){.type = VAL_FLOAT, .d = x}
#define FROM_CSTR(x) (value_t){.type = VAL_STR, .s = x}
#define FROM_FUNC(x) (value_t){.type = VAL_FUNC, .fn = x}

#define AS_INT(x) (x).i
#define AS_BOOL(x) (x).i
#define AS_FUNC(x) (x).fn
#define AS_FLOAT(x) (x).d

#define IS_BOOL(x) (x).type == VAL_BOOL
#define IS_INT(x) (x).type == VAL_INT
#define IS_FLOAT(x) (x).type == VAL_FLOAT

void value_destroy(value_t val);

function_t *function_native_new(melon_c_func func);
function_t *function_new(const char *identifier);
void function_free(function_t *func);
value_t function_cpool_get(function_t *func, int idx);
void function_cpool_dump(function_t *func);
void function_disassemble(function_t *func);


#endif