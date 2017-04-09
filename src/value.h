#ifndef __VALUE__
#define __VALUE__

#include "vector.h"

typedef enum
{
    VAL_BOOL, VAL_INT, VAL_FLOAT, VAL_STR
} value_e;


typedef struct
{
    value_e type;
    union
    {
        int i;
        float f;
        const char* o;
    };
} value_t;

typedef vector_t(value_t) value_r;

#define FROM_BOOL(x) (value_t){.type = VAL_BOOL, .i = x}
#define FROM_INT(x) (value_t){.type = VAL_INT, .i = x}
#define FROM_FLOAT(x) (value_t){.type = VAL_FLOAT, .f = x}
#define FROM_CSTR(x) (value_t){.type = VAL_STR, .o = x}

#define AS_INT(x) (x).i
#define AS_BOOL(x) (x).i

#endif