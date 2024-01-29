#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringvector.h"

#define STACK_SIZE 100

typedef struct {
    int* locals;
    int* globals;
    StringVector* code;
    int* stack;
    int pc;
    int sp;
    int fp;
    int lc;
    int gc;
} VM;


VM* init(StringVector* code, int datasize) {
    VM* vm = malloc(sizeof(VM));
    vm->code = code;
    vm->pc = 0;
    vm->fp = 0;
    vm->sp = -1;
    vm->lc = -1;
    vm->gc = -1;
    vm->stack = malloc(sizeof(int) * STACK_SIZE);
    vm->locals = malloc(sizeof(int) * datasize);
    vm->globals = malloc(sizeof(int) * datasize);
    return vm;
}

void destroy(VM* vm) {
    free(vm->locals);
    free(vm->globals);
    free(vm->stack);
    free(vm);
}

void push(VM* vm, int value) {
    vm->stack[++vm->sp] = value;
}

int pop(VM* vm) {
    return vm->stack[vm->sp--];
}

char* getNext(VM* vm) {
    return get(vm->code, vm->pc++);
}

void print_array(char* array_label, int* array, int array_size) {
    if (array_size == -1) {
        printf("%s: []\n", array_label);
        return;
    }
    printf("%s: [", array_label);
    for (int i = 0; i < array_size; i++) {
        printf("%d, ", array[i]);
    }
    printf("%d]\n", array[array_size]);

}

void display(VM* vm) {
    printf("pc: %d, fp: %d, sp: %d\n", vm->pc, vm->fp, vm->sp);
    print_array("Stack", vm->stack, vm->sp);
    print_array("Locals", vm->locals, vm->lc);
    print_array("Globals", vm->globals, vm->gc);
}

void run(VM* vm) {
    printf("Running program...\n");
    char* opcode;
    int len, value, addr, lhs, rhs;
    //int offset, argc, rval;
    while (1) {
        opcode = getNext(vm);
        len = strlen(opcode);
        if (strncmp(opcode, "HALT", len) == 0) {
            printf("Program execution complete\n");
            return; // stop program
        }
        else if (strncmp(opcode, "LOAD_CONST", len) == 0) {
            value = atoi(getNext(vm));
            push(vm, value);
        }
        else if (strncmp(opcode, "DUP", len) == 0) {
            if (vm->sp == -1)
                return;
            value = vm->stack[vm->sp];
            push(vm, value);
        }
        else if (strncmp(opcode, "ADD", len) == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            push(vm, lhs + rhs);
        }
        else if (strncmp(opcode, "MUL", len) == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            push(vm, lhs * rhs);

        }
        else if (strncmp(opcode, "EQ", len) == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            push(vm, (lhs == rhs) ? 1 : 0);
        }
        else if (strncmp(opcode, "STORE", len) == 0) {
            if (vm->sp == -1)
                return;
            value = pop(vm);
            vm->locals[++vm->lc] = value;
        }
        else if (strncmp(opcode, "LOAD", len) == 0) {
            addr = atoi(getNext(vm));
            value = vm->locals[addr];
            push(vm, value);
        }
        else if (strncmp(opcode, "GSTORE", len) == 0) {
            if (vm->sp == -1)
                return;
            value = pop(vm);
            vm->globals[++vm->gc] = value;
        }
        else if (strncmp(opcode, "GLOAD", len) == 0) {
            addr = atoi(getNext(vm));
            value = vm->globals[addr];
            push(vm, value);
        }
        else if (strncmp(opcode, "JMPT", len) == 0) {
            addr = atoi(getNext(vm));
            if (pop(vm))
                vm->pc = addr;
        }
        else if (strncmp(opcode, "JMPF", len) == 0) {
            addr = atoi(getNext(vm));
            printf("addr: %d\n", addr);
            if (!pop(vm))
                vm->pc = addr;
            printf("Jump to: %d(%s)\n", vm->pc, get(vm->code, addr));
        }
        else if (strncmp(opcode, "SELECT", len) == 0) {
            addr = atoi(getNext(vm));
            if (pop(vm))
                value = atoi(get(vm->code, vm->pc++));
            else {
                vm->pc += 2;
                value = atoi(get(vm->code, vm->pc));
            }
            push(vm, value);
        }
        else {
            printf("Unknown bytecode: %s\n", opcode);
            break;
        }
        display(vm);
    }
}

StringVector* read_file() {
    FILE* fptr;
    fptr = fopen("input.txt", "r");
    StringVector* line;
    StringVector* out = createStringVector();
    const unsigned int MAX_LENGTH = 256;
    char buff[MAX_LENGTH];
    printf("Reading file...\n");
    while (fgets(buff, MAX_LENGTH, fptr)) {
        line = split(buff, " ");
        trim(line);
        out = concat(out, line);
        free(line);
    }
    fclose(fptr);
    return out;
}

int main(int argc, char** argv) {
    StringVector* code = read_file();
    printStringVector(code);
    VM* vm = init(code, 100);
    run(vm);
    free(code);
    destroy(vm);
    return 0;
}