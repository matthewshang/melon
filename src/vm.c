#include "vm.h"

#include <stddef.h>

#include "core.h"
#include "debug.h"
#include "opcodes.h"

#define VM_GLOBALS_SIZE 2048
#define VM_STACK_SIZE 8

#define READ_BYTE       *vm->ip++
#define STACK_POP       *(--vm->stacktop)
#define STACK_PUSH(x)   stack_push(vm, x)
#define STACK_PEEK      *(vm->stacktop - 1)
#define STACK_POPN(n)   vm->stacktop -= (n)

#define RUNTIME_ERROR(...)                                                           \
        do {                                                                         \
            printf("Runtime error: ");                                               \
            printf(__VA_ARGS__);                                                     \
            return;                                                                  \
        } while (0)

#define CALL_FUNC(_cl, _bp, _nargs)                                                  \
        do {                                                                         \
            if (_cl->f->type == FUNC_MELON)                                          \
            {                                                                        \
                callstack_push(&vm->callstack, vm->ip, vm->closure, vm->bp);         \
                vm->bp = _bp;                                                        \
                vm->closure = _cl;                                                   \
                vm->ip = &vector_get(_cl->f->bytecode, 0);                           \
            }                                                                        \
            else if (_cl->f->type == FUNC_NATIVE)                                    \
            {                                                                        \
                value_t *adr = vm->stacktop - 1;                                     \
                adr -= _nargs == 0 ? 0 : _nargs - 1;                                 \
                _cl->f->cfunc(vm, adr, _nargs, adr - vm->stack - 1);                 \
                STACK_POPN(_nargs);                                                  \
            }                                                                        \
        } while (0)                                     

#define INT_BIN_MATH(a, b, op) do {STACK_PUSH(FROM_INT(a op b)); break; } while (0)
#define FLT_BIN_MATH(a, b, op) do {STACK_PUSH(FROM_FLOAT(a op b)); break; } while (0)
#define BOOL_BIN_MATH(a, b, op) do {STACK_PUSH(FROM_BOOL(a op b)); break; } while (0)

#define DO_FAST_BIN_MATH(op)                                                         \
        do {                                                                         \
            value_t b = STACK_POP, a = STACK_POP;                                    \
            if (IS_INT(a))                                                           \
            {                                                                        \
                if (IS_INT(b)) INT_BIN_MATH(a.i, b.i, op);                           \
                else if (IS_FLOAT(b)) FLT_BIN_MATH((double)a.i, b.d, op);            \
            }                                                                        \
            else if (IS_FLOAT(a))                                                    \
            {                                                                        \
                if (IS_INT(b)) FLT_BIN_MATH(a.d, (double)b.i, op);                   \
                else if (IS_FLOAT(b)) FLT_BIN_MATH(a.d, b.d, op);                    \
            }                                                                        \
        } while (0)

#define DO_FAST_INT_MATH(op)                                                         \
        do {                                                                         \
            value_t b = STACK_POP, a = STACK_POP;                                    \
            if (IS_INT(a) && IS_INT(b))                                              \
                INT_BIN_MATH(a.i, b.i, op);                                          \
        } while (0)

#define DO_FAST_BOOL_MATH(op)                                                        \
        do {                                                                         \
            value_t b = STACK_POP, a = STACK_POP;                                    \
            BOOL_BIN_MATH(a.i, b.i, op);                                             \
        } while (0)                 

#define DO_FAST_CMP_MATH(op)                                                         \
        do {                                                                         \
            value_t b = STACK_POP, a = STACK_POP;                                    \
            if (IS_INT(a))                                                           \
            {                                                                        \
                if (IS_INT(b)) BOOL_BIN_MATH(a.i, b.i, op);                          \
                else if (IS_FLOAT(b)) BOOL_BIN_MATH((double)a.i, b.d, op);           \
            }                                                                        \
            else if (IS_FLOAT(a))                                                    \
            {                                                                        \
                if (IS_INT(b)) BOOL_BIN_MATH(a.d, (double)b.i, op);                  \
                else if (IS_FLOAT(b)) BOOL_BIN_MATH(a.d, b.d, op);                   \
            }                                                                        \
        } while (0)

void callstack_push(callstack_t *stack, uint8_t *ret, closure_t *closure, uint16_t bp)
{
    callframe_t newframe;
    newframe.ret = ret;
    newframe.closure = closure;
    newframe.bp = bp;
    vector_push(callframe_t, *stack, newframe);
}

uint8_t *callstack_ret(callstack_t *stack, closure_t **closure, uint16_t *bp)
{
    callframe_t frame = vector_pop(*stack);
    uint8_t *ret = frame.ret;
    *closure = frame.closure;
    *bp = frame.bp;
    return ret;
}

void callstack_print(callstack_t stack)
{
    printf("Printing callstack %d\n", vector_size(stack));
    for (size_t i = 0; i < vector_size(stack); i++)
    {
        callframe_t frame = vector_get(stack, i);
        printf("frame - bp: %d, func: %s\n", frame.bp, frame.closure->f->identifier);
    }
}

vm_t vm_create()
{
    vm_t vm;
    vm.stack = (value_t*)calloc(VM_STACK_SIZE, sizeof(value_t));
    vm.stacksize = VM_STACK_SIZE;
    vm.stacktop = vm.stack;
    vm.upvalues = NULL;
    vector_init(vm.globals);
    vector_realloc(value_t, vm.globals, VM_GLOBALS_SIZE);
    vm.bp = 0;
    vm.ip = NULL;

    core_register_vm(&vm);

    vector_init(vm.callstack);
    vector_init(vm.mem);
    return vm;
}

void vm_set_global(vm_t *vm, value_t val, uint16_t idx)
{
    vector_set(vm->globals, idx, val);
}

void vm_set_stack(vm_t *vm, value_t val, uint32_t idx)
{
    vm->stack[idx] = val;
}

void vm_push_mem(vm_t *vm, value_t v)
{
    vector_push(value_t, vm->mem, v);
}

void vm_destroy(vm_t *vm)
{
    free(vm->stack);
    vector_destroy(vm->globals);
    vector_destroy(vm->callstack);
    printf("Allocated: %d\n", vector_size(vm->mem));
    for (size_t i = 0; i < vector_size(vm->mem); i++)
    {
        value_destroy(vector_get(vm->mem, i));
    }
    vector_destroy(vm->mem);

    upvalue_t *upvalue = vm->upvalues;
    while (upvalue)
    {
        upvalue_t *next = upvalue->next;
        free(upvalue);
        upvalue = next;
    }
}

upvalue_t *capture_upvalue(upvalue_t **upvalues, value_t *value)
{
    upvalue_t *prevup = NULL;
    upvalue_t *upvalue = *upvalues;
    while (upvalue && value > upvalue->value)
    {
        prevup = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue && upvalue->value == value) return upvalue;

    upvalue_t *newup = upvalue_new(value);
    
    if (prevup) prevup->next = newup;
    else *upvalues = newup;

    newup->next = upvalue;
    return newup;
}

static void close_upvalues(upvalue_t **upvalues, value_t *start)
{
    upvalue_t *upvalue = *upvalues;
    while (upvalue && upvalue->value >= start)
    {
        upvalue->closed = *upvalue->value;
        upvalue->value = &upvalue->closed;
        *upvalues = upvalue->next;
        upvalue = upvalue->next;
    }
}

static void stack_dump(vm_t *vm)
{
    printf("----Dumping stack----\n");
    value_t *v = vm->stack;
    while (v < vm->stacktop)
    {
        value_print(*v++);
    }
}

static void stack_push(vm_t *vm, value_t value)
{
    if (vm->stacktop - vm->stack == vm->stacksize)
    {
        value_t *old = vm->stack;
        vm->stack = (value_t*)realloc(vm->stack, sizeof(value_t) * vm->stacksize << 1);
        vm->stacktop = vm->stack + vm->stacksize;
        vm->stacksize = vm->stacksize << 1;

        if (old != vm->stack)
        {
            ptrdiff_t diff = vm->stack - old;
            upvalue_t *upvalue = vm->upvalues;
            while (upvalue)
            {
                upvalue->value += diff;
                upvalue = upvalue->next;
            }
        }
    }

    *vm->stacktop++ = value;
}

static void vm_run(vm_t *vm, bool is_main, uint32_t ret_bp, value_t **ret_val)
{
    uint8_t inst;

    while (1)
    {
        inst = *vm->ip++;
        switch ((opcode)inst)
        {
        case OP_RET0: 
        {
            close_upvalues(&vm->upvalues, &vm->stack[vm->bp]);
            STACK_POPN(vm->stacktop - vm->stack - vm->bp + 1);
            bool ret = !is_main && vm->bp == ret_bp;
            vm->ip = callstack_ret(&vm->callstack, &vm->closure, &vm->bp);
            if (ret) return;
            break;
        }
        case OP_NOP: continue;

        case OP_LOADL: STACK_PUSH(vm->stack[vm->bp + READ_BYTE]); break;
        case OP_LOADI: STACK_PUSH(FROM_INT(READ_BYTE)); break;
        case OP_LOADK: 
        {
            value_t val = function_cpool_get(vm->closure->f, READ_BYTE);
            STACK_PUSH(val); 
            if (IS_CLASS(val))
            {
                STACK_PUSH(val);
                class_t *c = AS_CLASS(val);
                if (c->meta_inited || !c->metaclass) break;
                c->static_vars = (value_t*)calloc(c->metaclass->nvars, sizeof(value_t));
                c->meta_inited = true;
                closure_t *init = class_lookup_closure(c->metaclass, FROM_CSTR("$init"));
                if (init)
                {
                    CALL_FUNC(init, vm->stacktop - vm->stack - 1, 0);
                }
            }
            break;
        }
        case OP_LOADU: 
        {
            STACK_PUSH(*vm->closure->upvalues[READ_BYTE]->value);
            break;
        }
        case OP_LOADF:
        {
            value_t accessor = STACK_POP;
            bool keepobj = READ_BYTE;
            value_t object = STACK_POP;
            value_t *index = NULL;
            if (IS_INT(accessor))
            {
                index = &accessor;
            }
            else
            {
                index = class_lookup_super(value_get_class(object), accessor);
            }

            if (!index)
            {
                RUNTIME_ERROR("class %s does not have property %s\n",
                    value_get_class(object)->identifier, AS_STR(accessor));
            }

            if (IS_INT(*index))
            {
                if (IS_CLASS(object))
                    STACK_PUSH(AS_CLASS(object)->static_vars[AS_INT(*index)]);
                else if (IS_INSTANCE(object))
                    STACK_PUSH(AS_INSTANCE(object)->vars[AS_INT(*index)]);
                else
                    RUNTIME_ERROR("tried to access an instance variable of non-instance object\n");
            }
            else
            {
                STACK_PUSH(*index);
            }

            if (keepobj) STACK_PUSH(object);
            break;
        }
        case OP_LOADG: STACK_PUSH(vector_get(vm->globals, READ_BYTE)); break;
        case OP_STOREL: vm->stack[vm->bp + READ_BYTE] = STACK_PEEK; break;
        case OP_STOREU: 
        {
            *vm->closure->upvalues[READ_BYTE]->value = STACK_PEEK;
            break;
        }
        case OP_STOREF:
        {
            value_t accessor = STACK_POP;
            value_t object = STACK_POP;
            bool is_class = IS_CLASS(object);
            value_t tostore = STACK_PEEK;
            value_t *index;
            if (IS_INT(accessor))
            {
                index = &accessor;
            }
            else
            {
                index = class_lookup_super(value_get_class(object), accessor);
            }

            if (!index)
            {
                RUNTIME_ERROR("class %s does not have property %s\n",
                    value_get_class(object)->identifier, AS_STR(accessor));
            }

            if (IS_CLASS(object))
            {
                AS_CLASS(object)->static_vars[AS_INT(*index)] = tostore;
            }
            else if (IS_INSTANCE(object))
            {
                AS_INSTANCE(object)->vars[AS_INT(*index)] = tostore;
            }
            else
            {
                RUNTIME_ERROR("tried to modify an instance variable of non-instance object\n");
            }

            break;
        }
        case OP_STOREG: vector_set(vm->globals, READ_BYTE, STACK_PEEK); break;

        case OP_CLOSURE: 
        {
            function_t *f = AS_CLOSURE(STACK_POP)->f;
            closure_t *newclose = closure_new(f);
            newclose->upvalues = (upvalue_t**)calloc(f->nupvalues, sizeof(upvalue_t*));

            for (uint8_t i = 0; i < f->nupvalues; i++)
            {
                uint8_t newup = READ_BYTE;
                if (newup != OP_NEWUP)
                    RUNTIME_ERROR("expected instruction NEWUP\n");

                uint8_t is_direct = READ_BYTE;
                newclose->upvalues[i] = is_direct ? 
                    capture_upvalue(&vm->upvalues, &vm->stack[vm->bp + READ_BYTE]) : vm->closure->upvalues[READ_BYTE];

            }
            STACK_PUSH(FROM_CLOSURE(newclose));
            break;
        }
        case OP_CALL:
        {
            uint8_t nargs = READ_BYTE;
            value_t v = *(vm->stacktop - nargs - 1);
            if (IS_CLASS(v))
            {
                class_t *c = AS_CLASS(v);
                value_t instance = FROM_INSTANCE(instance_new(c));
                vm_push_mem(vm, instance);

                closure_t *init = class_lookup_closure(c, FROM_CSTR("$init"));
                if (!init) RUNTIME_ERROR("missing init function in class %s\n", c->identifier);

                CALL_FUNC(init, vm->stacktop - vm->stack - nargs - 1, nargs);
                vm->stack[vm->bp] = instance;

                break;
            }
            if (!IS_CLOSURE(v)) 
                RUNTIME_ERROR("cannot call non-class or non-closure\n");

            closure_t *cl = AS_CLOSURE(v);
            CALL_FUNC(cl, vm->stacktop - vm->stack - nargs, nargs);
            break;
        }
        case OP_JMP: vm->ip += *vm->ip; break;
        case OP_LOOP: vm->ip -= *vm->ip; break;
        case OP_JIF: 
        {
            if (!AS_BOOL(STACK_POP)) vm->ip += *vm->ip;
            else vm->ip++;

            break;
        }
        case OP_RETURN:
        {
            close_upvalues(&vm->upvalues, &vm->stack[vm->bp]);
            vm->stack[vm->bp - 1] = STACK_PEEK;

            bool ret = !is_main && vm->bp == ret_bp;
            if (ret && ret_val) *ret_val = &vm->stack[vm->bp - 1];

            STACK_POPN(vm->stacktop - vm->stack - vm->bp);
            vm->ip = callstack_ret(&vm->callstack, &vm->closure, &vm->bp);

            if (ret) return;
            break;
        }

        case OP_ADD: DO_FAST_BIN_MATH(+); break;
        case OP_SUB: DO_FAST_BIN_MATH(-); break;
        case OP_MUL: DO_FAST_BIN_MATH(*); break;
        case OP_DIV: DO_FAST_BIN_MATH(/ ); break;
        case OP_MOD: DO_FAST_INT_MATH(%); break;
      
        case OP_AND: DO_FAST_BOOL_MATH(&&); break;
        case OP_OR: DO_FAST_BOOL_MATH(||); break;

        case OP_LT: DO_FAST_CMP_MATH(<); break;
        case OP_GT: DO_FAST_CMP_MATH(>); break;
        case OP_LTE: DO_FAST_CMP_MATH(<= ); break;
        case OP_GTE: DO_FAST_CMP_MATH(>= ); break;
        case OP_EQ: DO_FAST_CMP_MATH(== ); break;
        case OP_NEQ: DO_FAST_CMP_MATH(!= ); break;

        case OP_NOT: 
        {
            value_t val = STACK_POP;
            if (IS_BOOL(val)) STACK_PUSH(FROM_BOOL(!val.i));
            break;
        }
        case OP_NEG: 
        {
            value_t val = STACK_POP;                               
            if (IS_INT(val)) STACK_PUSH(FROM_INT(-val.i));
            else if (IS_FLOAT(val)) STACK_PUSH(FROM_FLOAT(-val.d));
            break;
        }

        case OP_NEWARR:
        {
            array_t *a = array_new();
            uint8_t len = READ_BYTE;
            for (uint8_t i = 0; i < len; i++)
            {
                vector_push(value_t, a->arr, *(vm->stacktop - len + i));
            }
            value_t a_val = FROM_ARRAY(a);
            vm_push_mem(vm, a_val);
            STACK_PUSH(a_val);
            break;
        }

        case OP_HALT: return;
        default: continue;

        }
    }
}

void vm_run_main(vm_t *vm, function_t *main)
{
    closure_t *cl = closure_new(main);
    vm->closure = cl;
    vm->ip = &vector_get(cl->f->bytecode, 0);

    vm_run(vm, true, 0, NULL);

    free(cl);
}

void vm_run_closure(vm_t *vm, closure_t *cl, value_t args[], uint16_t nargs, value_t **ret)
{
    if (cl->f->type == FUNC_MELON)
    {
        for (uint16_t i = 0; i < nargs; i++)
        {
            stack_push(vm, args[i]);
        }

        CALL_FUNC(cl, vm->stacktop - vm->stack - nargs, nargs);
        vm_run(vm, false, vm->bp, ret);
    }
    else if (cl->f->type == FUNC_NATIVE)
    {
        cl->f->cfunc(vm, args, nargs, vm->stacktop - vm->stack - 1);
    }
}