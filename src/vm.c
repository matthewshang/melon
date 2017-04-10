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

vm_t vm_create(byte_r code, value_r constants)
{
    vm_t vm;
    vector_copy(value_t, constants, vm.constants);
    vector_init(vm.stack);
    vector_copy(uint8_t, code, vm.bytecode);
    vm.ip = &vector_get(vm.bytecode, 0);
    return vm;
}

void vm_destroy(vm_t *vm)
{
    vector_destroy(vm->stack);
    for (int i = 0; i < vector_size(vm->constants); i++)
    {
        if (vector_get(vm->constants, i).type == VAL_STR)
        {
            free(vector_get(vm->constants, i).o);
        }
    }
    vector_destroy(vm->bytecode);
    vector_destroy(vm->constants);
}

static void stack_dump(value_r *stack)
{
    printf("----Dumping stack----\n");
    for (int i = 0; i < vector_size(*stack); i++)
    {
        value_t v = vector_get(*stack, i);
        switch (v.type)
        {
        case VAL_BOOL: printf("%s\n", v.i == 1 ? "true" : "false"); break;
        case VAL_INT: printf("%d\n", v.i); break;
        case VAL_STR: printf("%s\n", v.o); break;
        default: break;
        }
    }
}

void vm_run(vm_t *vm)
{
    uint8_t inst;

    while (1)
    {
        inst = *vm->ip++;
        switch ((opcode)inst)
        {
        case OP_RET0: break;
        case OP_NOP: continue;

        case OP_LOAD: STACK_PUSH(vector_get(vm->stack, READ_BYTE)); break;
        case OP_LOADI: STACK_PUSH(FROM_INT(READ_BYTE)); break;
        case OP_LOADK: STACK_PUSH(vector_get(vm->constants, READ_BYTE)); break;
        case OP_STORE: vector_set(vm->stack, READ_BYTE, STACK_PEEK); break;

        case OP_CALL: {

            value_t v = STACK_POP;
            switch (v.type)
            {
            case VAL_BOOL: printf("%s\n", v.i == 1 ? "true" : "false"); break;
            case VAL_INT: printf("%d\n", v.i); break;
            case VAL_STR: printf("%s\n", v.o); break;
            default: break;
            }
            break;
        }
        case OP_JMP: vm->ip += *vm->ip; break;
        case OP_LOOP: vm->ip -= *vm->ip; break;
        case OP_JIF: {
            if (!AS_BOOL(STACK_POP)) vm->ip += *vm->ip;
            else vm->ip++;

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
        //stack_dump(&vm->stack);
        //printf("\n");

    }
}

void vm_dump_constants(vm_t *vm)
{
    printf("----Dumping VM constants----\n");
    for (int i = 0; i < vector_size(vm->constants); i++)
    {
        value_t v = vector_get(vm->constants, i);
        switch (v.type)
        {
        case VAL_BOOL: printf("\t%s\n", v.i == 1 ? "true" : "false"); break;
        case VAL_INT: printf("\t%d\n", v.i); break;
        case VAL_STR: printf("\t%s\n", v.o); break;
        default: break;
        }
    }
    printf("\n");
}