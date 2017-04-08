#include "vm.h"

#include "debug.h"
#include "opcodes.h"

#define READ_BYTE       *vm->ip++
#define STACK_POP       vector_pop(vm->stack)
#define STACK_PUSH(x)   vector_push(value_t, vm->stack, x)
#define STACK_PEEK      vector_peek(vm->stack)
#define BINARY_OP(op)   do {                                                         \
                            int b = AS_INT(STACK_POP), a = AS_INT(STACK_POP);        \
                            STACK_PUSH(FROM_INT(a op b));                            \
                        } while (0)                                                  \

#define UNARY_OP(op)    do {                                                         \
                            int a = AS_INT(STACK_POP);                               \
                            STACK_PUSH(FROM_INT(op a));                              \
                        } while (0)                                                  \

vm_t vm_create(vector_t(uint8_t) *code, value_r constants)
{
    vm_t vm;
    vm.constants = constants;
    vector_init(vm.stack);
    vm.ip = &vector_get(*code, 0);
    return vm;
}

void vm_destroy(vm_t *vm)
{
    vector_destroy(vm->stack);
}

static void stack_dump(value_r *stack)
{
    printf("Dumping stack\n");
    for (int i = 0; i < vector_size(*stack); i++)
    {
        value_t v = vector_get(*stack, i);
        switch (v.type)
        {
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
            case VAL_INT: printf("%d\n", v.i); break;
            case VAL_STR: printf("%s\n", v.o); break;
            default: break;
            }
            break;
        }
        case OP_JMP: vm->ip += *vm->ip; break;
        case OP_LOOP: vm->ip -= *vm->ip; break;
        case OP_JIF: {
            if (!AS_INT(STACK_POP)) vm->ip += *vm->ip;
            else vm->ip++;

            break;
        }

        case OP_ADD: BINARY_OP(+); break;
        case OP_SUB: BINARY_OP(-); break;
        case OP_MUL: BINARY_OP(*); break;
        case OP_MOD: BINARY_OP(%); break;
      
        case OP_AND: BINARY_OP(&&); break;
        case OP_OR: BINARY_OP(||); break;
        case OP_LT: BINARY_OP(<); break;
        case OP_GT: BINARY_OP(>); break;
        case OP_NOT: UNARY_OP(!); break;
        case OP_NEG: UNARY_OP(-); break;
        case OP_EQ: BINARY_OP(==); break;
        case OP_NEQ: BINARY_OP(!=); break;

        case OP_HALT: return;
        default: continue;

        }

        //printf("Instruction: %s\n", op_to_str(inst));
        //stack_dump(&vm->stack);
        //printf("\n");

    }
}