#ifndef __VALUE__
#define __VALUE__

#include <stdbool.h>

#include "vector.h"

typedef enum
{
    VAL_BOOL, VAL_INT, VAL_FLOAT, VAL_STR, VAL_CLOSURE, VAL_CLASS, VAL_INST
} value_e;

typedef struct closure_s closure_t;

typedef struct class_s class_t;
typedef struct class_s object_t;

class_t *melon_class_object;
class_t *melon_class_class;
class_t *melon_class_bool;
class_t *melon_class_int;
class_t *melon_class_float;
class_t *melon_class_string;
class_t *melon_class_closure;
class_t *melon_class_instance;

typedef struct instance_s instance_t;

typedef struct
{
    class_t *type;
    union
    {
        int i;
        double d;
        const char *s;
        closure_t *cl;
        class_t *c;
        instance_t *inst;
    };
} value_t;

typedef vector_t(value_t) value_r;

typedef enum
{
    FUNC_MELON, FUNC_NATIVE
} function_e;

typedef struct vm_s vm_t;
typedef void(*melon_c_func)(vm_t *vm, value_t *args, uint8_t nargs, uint32_t retidx);

typedef struct function_s
{
    function_e type;
    uint8_t nupvalues;
    
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

struct hashtable_t;
typedef struct class_s
{
    const char *identifier;
    class_t *superclass;

    struct hashtable_t *htable;
    uint16_t nvars;

    class_t *metaclass;
    bool meta_inited;
    value_t *static_vars;

} class_s;

typedef struct instance_s
{
    class_t *c;
    uint16_t nvars;
    value_t *vars;

} instance_s;

typedef struct upvalue_s
{
    value_t *value;
    value_t closed;
    struct upvalue_s *next;
} upvalue_t;

typedef struct closure_s
{
    function_t *f;
    upvalue_t **upvalues;
} closure_t;

#define FROM_BOOL(x) (value_t){.type = melon_class_bool, .i = x}
#define FROM_INT(x) (value_t){.type = melon_class_int, .i = x}
#define FROM_FLOAT(x) (value_t){.type = melon_class_float, .d = x}
#define FROM_CSTR(x) (value_t){.type = melon_class_string, .s = x}
#define FROM_CLOSURE(x) (value_t){.type = melon_class_closure, .cl = x}
#define FROM_CLASS(x) (value_t){.type = melon_class_class, .c = x}
#define FROM_INSTANCE(x) (value_t){.type = melon_class_instance, .inst = x}

#define AS_INT(x) (x).i
#define AS_BOOL(x) (x).i
#define AS_STR(x) (x).s
#define AS_CLOSURE(x) (x).cl
#define AS_FLOAT(x) (x).d
#define AS_CLASS(x) (x).c
#define AS_INSTANCE(x) (x).inst

#define IS_BOOL(x) ((x).type == melon_class_bool)
#define IS_INT(x) ((x).type == melon_class_int)
#define IS_FLOAT(x) ((x).type == melon_class_float)
#define IS_STR(x) ((x).type == melon_class_string)
#define IS_CLOSURE(x) ((x).type == melon_class_closure)
#define IS_CLASS(x) ((x).type == melon_class_class)
#define IS_INSTANCE(x) ((x).type == melon_class_instance)

void value_destroy(value_t val);
void value_print(value_t val);
bool value_equals(value_t v1, value_t v2);

function_t *function_native_new(melon_c_func func);
function_t *function_new(const char *identifier);
void function_free(function_t *func);
value_t function_cpool_get(function_t *func, int idx);
void function_cpool_dump(function_t *func);
void function_disassemble(function_t *func);

upvalue_t *upvalue_new(value_t *value);
void upvalue_free(upvalue_t *upvalue);

closure_t *closure_new(function_t *func);
void closure_free(closure_t *closure);

class_t *class_new(const char *identifier, uint16_t nvars, class_t *superclass);
class_t *class_new_with_meta(const char *identifier, uint16_t nvars, uint16_t nstatic, class_t *superclass);
void class_free(class_t *c);
void class_print(class_t *c);
void class_set_superclass(class_t *c, class_t *super);
void class_bind(class_t *c, value_t key, value_t value);
value_t *class_lookup(class_t *c, value_t key);
value_t *class_lookup_super(class_t *c, value_t key);
closure_t *class_lookup_closure(class_t *c, value_t key);

instance_t *instance_new(class_t *c);
void instance_free(instance_t *inst);

#endif