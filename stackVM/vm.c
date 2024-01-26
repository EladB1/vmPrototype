#include <stdio.h>
#include <stdlib.h>
#include "bytecode.h"

#define STACK_SIZE 100
#define PUSH(vm, v) vm->stack[++vm->sp] = v // push value on top of the stack
#define POP(vm) vm->stack[vm->sp--] // pop value from top of stack
#define NEXT(vm) vm->code[vm->pc++] // get next bytecode


typedef struct {
    int* locals; // local scoped data
    int* code; // array op byte codes to be executed
    int* stack; 
    int pc; // program counter (AKA instruction pointer)
    int sp; // stack pointer
    int fp; // frame pointer (for local scope)
} VM;

VM* initVM(int* code, int pc, int datasize) {
    VM* vm = malloc(sizeof(VM));
    vm->code = code;
    vm->pc = pc;
    vm->fp = 0;
    vm->sp = -1;
    vm->locals = malloc(sizeof(int) * datasize);
    vm->stack = malloc(sizeof(int) * STACK_SIZE);
    return vm;
}

void destroyVM(VM* vm) {
    free(vm->locals);
    free(vm->stack);
    free(vm);
}

void run(VM* vm) {
    printf("Running program...\n");
    do {
        int opcode = NEXT(vm);
        int value, addr, offset, lhs, rhs, argc, rval;

        switch (opcode) {
            case HALT:
                return; // stop program
            case CONST_I32:
                value = NEXT(vm); // get next value in stack
                PUSH(vm, value); // push it to the top
                break;
            case ADD_I32:
                rhs = POP(vm);
                lhs = POP(vm);
                PUSH(vm, lhs + rhs);
                break;
            case SUB_I32:
                rhs = POP(vm);
                lhs = POP(vm);
                PUSH(vm, lhs - rhs);
                break;
            case MUL_I32:
                rhs = POP(vm);
                lhs = POP(vm);
                PUSH(vm, lhs * rhs);
                break;
            case LT_I32:
                rhs = POP(vm);
                lhs = POP(vm);
                PUSH(vm, (lhs < rhs) ? 1 : 0);
                break;
            case EQ_I32:
                rhs = POP(vm);
                lhs = POP(vm);
                PUSH(vm, (lhs == rhs) ? 1 : 0);
                break;
            case POP:
                --vm->sp; // throw away top of stack
                break;
            case PRINT:
                value = POP(vm);
                printf("%d\n", value);
                break;
            case JMP:
                vm->pc = NEXT(vm);
                break;
            case JMPT:
                addr = NEXT(vm); // get address pointer from code
                if (POP(vm)) // if value on top of stack is true
                    vm->pc = addr; // go to the address
                break;
            case JMPF:
                addr = NEXT(vm);
                if (!POP(vm))
                    vm->pc = addr;
                break;
            case LOAD:
                offset = NEXT(vm);
                PUSH(vm, vm->stack[vm->fp + offset]); // put on top of stack pointer relative to frame pointer
                break;
            case GLOAD:
                addr = POP(vm);
                value = vm->locals[addr];
                PUSH(vm, value);
                break;
            case STORE:
                value = POP(vm);
                offset = NEXT(vm);
                vm->locals[vm->fp + offset] = value;
                break;
            case GSTORE:
                value = POP(vm);
                addr = NEXT(vm);
                vm->locals[addr] = value;
                break;
            case CALL:
                // we expect all args to be on the stack
                addr = NEXT(vm); // get next instruction as address of procedure jump
                argc = NEXT(vm); // and next one as number of arguments to load
                PUSH(vm, argc); // save the number of args
                PUSH(vm, vm->fp); // save function pointer
                PUSH(vm, vm->pc); // save instruction pointer
                vm->fp = vm->sp; // set new frame pointer
                vm->pc = addr; // move instruction pointer to target procedure address
                break;
            case RET:
                rval = POP(vm); // pop return value from top of stack
                vm->sp = vm->fp; // return from procedure address
                vm->pc = POP(vm); // restore instruction pointer
                vm->fp = POP(vm); // restore frame pointer
                argc = POP(vm); // how many args procedure has
                vm->sp -= argc; // discard all args left
                PUSH(vm, rval);
                break;
            default:
                printf("Unknown bytecode\n");
                break;
        }
    } while (1);
    printf("Program execution complete\n");
}

int main(int argc, char** argv) {
    int code[] = {ADD_I32, 5, 6, HALT};
    VM* vm = initVM(code, 0, 100);
    run(vm);
    return 0;
}