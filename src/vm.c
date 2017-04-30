#include "vm.h"

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

#define UNAOP_INT(op)   do {                                                         \
                            int a = AS_INT(STACK_POP);                               \
                            STACK_PUSH(FROM_INT(op a));                              \
                        } while (0)                                                  \

#define BINCOMP(op)     do {                                                         \
                            int b = AS_INT(STACK_POP), a = AS_INT(STACK_POP);        \
                            STACK_PUSH(FROM_BOOL(a op b));                           \
                        } while (0)

#define INT_BIN_MATH(a, b, op) do {STACK_PUSH(FROM_INT(a op b)); break; } while (0)
#define FLT_BIN_MATH(a, b, op) do {STACK_PUSH(FROM_FLOAT(a op b)); break; } while (0)
#define BOOL_BIN_MATH(a, b, op) do {STACK_PUSH(FROM_BOOL(a op b)); break; } while (0)

#define DO_FAST_BIN_MATH(op)  do {                                                   \
                            value_t b = STACK_POP, a = STACK_POP;                    \
                            if (IS_INT(a))                                           \
                            {                                                        \
                                if (IS_INT(b)) INT_BIN_MATH(a.i, b.i, op);           \
                                else if (IS_FLOAT(b)) FLT_BIN_MATH((double)a.i, b.d, op); \
                            }                                                        \
                            else if (IS_FLOAT(a))                                    \
                            {                                                        \
                                if (IS_INT(b)) FLT_BIN_MATH(a.d, (double)b.i, op);   \
                                else if (IS_FLOAT(b)) FLT_BIN_MATH(a.d, b.d, op);    \
                            }                                                        \
                        } while (0)

#define DO_FAST_INT_MATH(op) do {                                                    \
                            value_t b = STACK_POP, a = STACK_POP;                    \
                            if (IS_INT(a) && IS_INT(b))                              \
                                INT_BIN_MATH(a.i, b.i, op);                          \
                        } while (0)

#define DO_FAST_BOOL_MATH(op) do {                                                   \
                            value_t b = STACK_POP, a = STACK_POP;                    \
                            BOOL_BIN_MATH(a.i, b.i, op);                             \
                        } while (0)                 

#define DO_FAST_CMP_MATH(op)  do {                                                   \
                            value_t b = STACK_POP, a = STACK_POP;                    \
                            if (IS_INT(a))                                           \
                            {                                                        \
                                if (IS_INT(b)) BOOL_BIN_MATH(a.i, b.i, op);          \
                                else if (IS_FLOAT(b)) BOOL_BIN_MATH((double)a.i, b.d, op); \
                            }                                                        \
                            else if (IS_FLOAT(a))                                    \
                            {                                                        \
                                if (IS_INT(b)) BOOL_BIN_MATH(a.d, (double)b.i, op);  \
                                else if (IS_FLOAT(b)) BOOL_BIN_MATH(a.d, b.d, op);   \
                            }                                                        \
                        } while (0)

void callstack_push(callframe_t **top, uint8_t *ret, closure_t *closure, uint16_t bp)
{
    callframe_t *newframe = (callframe_t*)calloc(1, sizeof(callframe_t));
    newframe->last = *top;
    newframe->ret = ret;
    newframe->closure = closure;
    newframe->bp = bp;
    *top = newframe;
}

uint8_t *callstack_ret(callframe_t **top, closure_t **closure, uint16_t *bp)
{
    callframe_t *frame = *top;
    *top = frame->last;
    uint8_t *ret = frame->ret;
    *closure = frame->closure;
    *bp = frame->bp;
    free(frame);
    return ret;
}

void callstack_print(callframe_t *top)
{
    printf("Printing callstack\n");
    callframe_t *prev = top;
    while (prev)
    {
        printf("frame - bp: %d, func: %s\n", prev->bp, prev->closure->f->identifier);
        prev = prev->last;
    }
}

vm_t vm_create(function_t *f)
{
    vm_t vm;
    vm.stack = (value_t*)calloc(VM_STACK_SIZE, sizeof(value_t));
    vm.stacksize = VM_STACK_SIZE;
    vm.stacktop = vm.stack;
    vm.main_func = f;
    vm.ip = &vector_get(vm.main_func->bytecode, 0);
    vector_init(vm.globals);
    vector_realloc(value_t, vm.globals, VM_GLOBALS_SIZE);

    core_register_vm(&vm);

    vm.callstack = NULL;
    return vm;
}

void vm_set_global(vm_t *vm, value_t val, uint16_t idx)
{
    vector_set(vm->globals, idx, val);
}

void vm_destroy(vm_t *vm)
{
    free(vm->stack);
    vector_destroy(vm->globals);
    callframe_t *frame = vm->callstack;
    while (frame)
    {
        callframe_t *prev = frame->last;
        free(frame);
        frame = prev;
    }
}

upvalue_t *capture_upvalue(upvalue_t **upvalues, value_t *value)
{
    upvalue_t *upvalue = upvalue_new(value);
    upvalue->next = *upvalues;
    *upvalues = upvalue;
    return upvalue;
}

static void close_upvalues(upvalue_t **upvalues, value_t *start)
{
    upvalue_t *upvalue = *upvalues;
    if (!upvalue) return;
    while (upvalue->value >= start)
    {
        upvalue->closed = *upvalue->value;
        upvalue->value = &upvalue->closed;
        value_print(*upvalue->value);
        *upvalues = upvalue->next;
        upvalue = upvalue->next;
        if (!upvalue) return;
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
        vm->stack = (value_t*)realloc(vm->stack, sizeof(value_t) * vm->stacksize << 1);
        vm->stacktop = vm->stack + vm->stacksize;
        vm->stacksize = vm->stacksize << 1;
    }

    *vm->stacktop++ = value;
}

void vm_run(vm_t *vm)
{
    uint8_t inst;
    closure_t *closure = closure_new(vm->main_func);
    uint16_t bp = 0;
    upvalue_t *upvalues = NULL;

    while (1)
    {
        inst = *vm->ip++;
        switch ((opcode)inst)
        {
        case OP_RET0: 
        {
            STACK_POPN((vm->stacktop - vm->stack) - bp);
            vm->ip = callstack_ret(&vm->callstack, &closure, &bp);
            break;
        }
        case OP_NOP: continue;

        case OP_LOAD: STACK_PUSH(vm->stack[bp + READ_BYTE]); break;
        case OP_LOADI: STACK_PUSH(FROM_INT(READ_BYTE)); break;
        case OP_LOADK: STACK_PUSH(function_cpool_get(closure->f, READ_BYTE)); break;
        case OP_LOADU: 
        {
            uint8_t read = READ_BYTE;
            value_t v = *closure->upvalues[read]->value;
            printf("loadu: ");
            value_print(v);
            printf("read: %d\n", read);
            STACK_PUSH(*closure->upvalues[read]->value); 
            break;
        }
        case OP_LOADG: STACK_PUSH(vector_get(vm->globals, READ_BYTE)); break;
        case OP_STORE: vm->stack[bp + READ_BYTE] = STACK_PEEK; break;
        case OP_STOREU: 
        {
            value_t v = STACK_PEEK;
            printf("storeu: ");  value_print(v);
            //*closure->upvalues[READ_BYTE]->value = v; 
            closure->upvalues[READ_BYTE]->value->i = v.i;
            //value_t *pos = &vm->stack[1];
            //vm->stack.a[1].i = v.i;
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
                {
                    printf("Runtime error: expected instruction NEWUP\n");
                    return;
                }
                //value_t *pos = &vm->stack.a[1];

                newclose->upvalues[i] = capture_upvalue(&upvalues, &vm->stack[bp + READ_BYTE]);
                newclose->upvalues[i]->value->i = 1;
                printf("wtf %d\n", newclose->upvalues[i]->value->i);
                value_t v = *newclose->upvalues[i]->value;
                printf("upvalue: %d", newclose->upvalues[i]->value->type);
                value_print(v);
            }
            STACK_PUSH(FROM_CLOSURE(newclose));
            break;
        }
        case OP_CALL:
        {
            closure_t *cl = AS_CLOSURE(STACK_POP);
            if (cl->f->type == FUNC_MELON)
            {
                callstack_push(&vm->callstack, vm->ip + 1, closure, bp);
                bp = vm->stacktop - vm->stack - READ_BYTE;
                closure = cl;
                vm->ip = &vector_get(cl->f->bytecode, 0);
            }
            else if (cl->f->type == FUNC_NATIVE)
            {
                uint8_t nargs = READ_BYTE;
                value_t *adr = vm->stacktop - 1;
                adr -= nargs == 0 ? 0 : nargs - 1;
                cl->f->cfunc(adr, nargs);
                STACK_POPN(nargs);
            }
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
            close_upvalues(&upvalues, &vm->stack[bp]);
            vm->stack[bp] = STACK_PEEK;
            STACK_POPN(vm->stacktop - vm->stack - bp - 1);
            vm->ip = callstack_ret(&vm->callstack, &closure, &bp);
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

        case OP_HALT: return;
        default: continue;

        }

        ////callstack_print(vm->callstack);
        //printf("Instruction: %s\n", op_to_str(inst));
        //printf("bp: %d\n", bp);
        //stack_dump(vm);
        //printf("\n");

    }
}