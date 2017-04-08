#include "value.h"

value_t value_from_int(int val)
{
    value_t value;
    value.type = VAL_INT;
    value.i = val;
    return value;
}

value_t value_from_float(float val)
{
    value_t value;
    value.type = VAL_FLOAT;
    value.f = val;
    return value;
}

value_t value_from_string(const char *val)
{
    value_t value;
    value.type = VAL_STRING;
    value.o = val;
    return value;
}

int val_as_int(value_t val)
{
    return val.i;
}
