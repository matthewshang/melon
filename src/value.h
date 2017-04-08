#ifndef __VALUE__
#define __VALUE__

#include "vector.h"

typedef enum
{
    VAL_INT, VAL_FLOAT, VAL_STRING
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

value_t value_from_int(int val);
value_t value_from_float(float val);
value_t value_from_string(const char* val);

int val_as_int(value_t val);

#endif