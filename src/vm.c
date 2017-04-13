#include "vm.h"

#include "debug.h"
#include "opcodes.h"

#define READ_BYTE       *vm->ip++
#define STACK_POP       vector_pop(vm->stack)
#define STACK_PUSH(x)   vector_push(value_t, vm->stack, x)
#define STACK_PEEK      vector_peek(vm->stack)  
#define BINOP_INT(op)   do {                                                         \
                            int b = AS_INT(STACK_POP), a = AS_INT(STACK_POP);        \
                            STACK_PUSH(FROM_INT(a op b));                            \
                        } while (0)                                                  \

#define UNAOP_INT(op)   do {                                                         \
                            int a = AS_INT(STACK_POP);                               \
                            STACK_PUSH(FROM_INT(op a));                              \
                        } while (0)                                                  \

#define BINCOMP(op)     do {                                                         \
                            int b = AS_INT(STACK_POP), a = AS_INT(STACK_POP);        \
                            STACK_PUSH(FROM_BOOL(a op b));                           \
                        } while (0)

void callstack_push(callframe_t **top, uint8_t *ret, function_t *func, uint16_t bp)
{
    callframe_t *newframe = (callframe_t*)calloc(1, sizeof(callframe_t));
    newframe->last = *top;
    newframe->ret = ret;
    newframe->func = func;
    newframe->bp = bp;
    *top = newframe;
}

uint8_t *callstack_ret(callframe_t **top, function_t **cur_func, uint16_t *bp)
{
    callframe_t *frame = *top;
    *top = frame->last;
    uint8_t *ret = frame->ret;
    *cur_func = frame->func;
    *bp = frame->bp;
    free(frame);
    return ret;
}

vm_t vm_create(function_t *f)
{
    vm_t vm;
    vector_init(vm.stack);
    vm.main_func = f;
    vm.ip = &vector_get(vm.main_func->bytecode, 0);
    vector_init(vm.globals);
    for (size_t i = 0; i < 10; i++) vector_push(value_t, vm.globals, FROM_INT(0));
    vm.callstack = (callframe_t*)calloc(1, sizeof(callframe_t));
    vm.callstack->ret = NULL;
    vm.callstack->last = NULL;
    vm.callstack->bp = 0;

    return vm;
}

void vm_destroy(vm_t *vm)
{
    vector_destroy(vm->stack);
    vector_destroy(vm->globals);
    callframe_t *frame = vm->callstack;
    while (frame)
    {
        callframe_t *prev = frame->last;
        free(frame);
        frame = prev;
    }
}

static void stack_dump(value_r *stack)
{
    printf("----Dumping stack----\n");
    for (int i = 0; i < vector_size(*stack); i++)
    {
        value_t v = vector_get(*stack, i);
        switch (v.type)
        {
        case VAL_BOOL: printf("[bool]: %s\n", v.i == 1 ? "true" : "false"); break;
        case VAL_INT: printf("[int]: %d\n", v.i); break;
        case VAL_STR: printf("[string]: %s\n", v.s); break;
        case VAL_FUNC: printf("[function]: %s\n", v.fn->identifier); break;
        default: break;
        }
    }
}

void vm_run(vm_t *vm)
{
    uint8_t inst;
    function_t *cur_func = vm->main_func;
    uint16_t bp = 0;

    while (1)
    {
        inst = *vm->ip++;
        switch ((opcode)inst)
        {
        case OP_RET0: 
        {
            vm->ip = callstack_ret(&vm->callstack, &cur_func, &bp);
            vector_popn(vm->stack, vector_size(vm->stack) - bp - 1);
            break;
        }
        case OP_NOP: continue;

        case OP_LOAD: STACK_PUSH(vector_get(vm->stack, bp + READ_BYTE)); break;
        case OP_LOADI: STACK_PUSH(FROM_INT(READ_BYTE)); break;
        case OP_LOADK: STACK_PUSH(function_cpool_get(cur_func, READ_BYTE)); break;
        case OP_LOADG: STACK_PUSH(vector_get(vm->globals, READ_BYTE)); break;
        case OP_STORE: vector_set(vm->stack, bp + READ_BYTE, STACK_PEEK); break;
        case OP_STOREG: vector_set(vm->globals, READ_BYTE, STACK_PEEK); break;

        case OP_PRINT: {

            value_t v = STACK_POP;
            switch (v.type)
            {
            case VAL_BOOL: printf("%s\n", v.i == 1 ? "true" : "false"); break;
            case VAL_INT: printf("%d\n", v.i); break;
            case VAL_STR: printf("%s\n", v.s); break;
            default: break;
            }
            break;
        }
        case OP_CALL:
        {
            function_t *f = AS_FUNC(STACK_POP);
            bp = vector_size(vm->stack) - READ_BYTE;
            callstack_push(&vm->callstack, vm->ip, cur_func, bp);
            cur_func = f;
            vm->ip = &vector_get(f->bytecode, 0);
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
            vector_set(vm->stack, bp, STACK_PEEK);
            vector_popn(vm->stack, vector_size(vm->stack) - bp - 1);
            vm->ip = callstack_ret(&vm->callstack, &cur_func, &bp);
            break;
        }

        case OP_ADD: BINOP_INT(+); break;
        case OP_SUB: BINOP_INT(-); break;
        case OP_MUL: BINOP_INT(*); break;
        case OP_MOD: BINOP_INT(%); break;
      
        case OP_AND: BINCOMP(&&); break;
        case OP_OR: BINCOMP(||); break;
        case OP_LT: BINCOMP(<); break;
        case OP_GT: BINCOMP(>); break;
        case OP_LTE: BINCOMP(<= ); break;
        case OP_GTE: BINCOMP(>= ); break;
        case OP_EQ: BINCOMP(== ); break;
        case OP_NEQ: BINCOMP(!= ); break;

        case OP_NOT: 
        {
            int v = AS_BOOL(STACK_POP);
            STACK_PUSH(FROM_BOOL(!v));
            break;
        }
        case OP_NEG: 
        {
            int v = AS_INT(STACK_POP);                               
            STACK_PUSH(FROM_INT(-v)); 
            break;
        }


        case OP_HALT: return;
        default: continue;

        }

        //printf("Instruction: %s\n", op_to_str(inst));
        //printf("bp: %d\n", bp);
        //stack_dump(&vm->stack);
        //printf("\n");

    }
}