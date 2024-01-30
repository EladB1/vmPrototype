#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "stringvector.h"
#include "stackmember.h"

#define STACK_SIZE 100

typedef struct {
    StackMember* locals;
    StackMember* globals;
    StringVector* code;
    StackMember* stack;
    int pc;
    int sp;
    int fp;
    int lc;
    int gc;
} VM;

bool constantIsInteger(char* constant) {
    for (int i = 0; i < strlen(constant); i++) {
        if (!isdigit(constant[i]))
            return false;
    }
    return true;
}

bool constantIsDouble(char* constant) {
    int dotCount = 0;
    for (int i = 0; i < strlen(constant); i++) {
        if (constant[i] == '.')
            dotCount++;
        else if (!isdigit(constant[i]))
            return false;
    }
    return dotCount == 1;
}

bool constantIsBoolean(char* constant) {
    int len = strlen(constant);
    return strncmp(constant, "0", len) == 0 || strncmp(constant, "1", len) == 0;
}


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

void push(VM* vm, StackMember value) {
    vm->stack[++vm->sp] = value;
}

StackMember pop(VM* vm) {
    return vm->stack[vm->sp--];
}

char* getNext(VM* vm) {
    return get(vm->code, vm->pc++);
}

void print_array(char* array_label, StackMember* array, int array_size) {
    if (array_size == -1) {
        printf("%s: []\n", array_label);
        return;
    }
    printf("%s: [", array_label);
    for (int i = 0; i < array_size; i++) {
        printf("%s, ", toString(array[i]));
    }
    printf("%s]\n", toString(array[array_size]));

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
    StackMember value, lhs, rhs, rval;
    char* next;
    int len, addr;
    //int offset, argc, rval;
    while (1) {
        opcode = getNext(vm);
        len = strlen(opcode);
        if (strncmp(opcode, "HALT", 5) == 0) {
            printf("Program execution complete\n");
            return; // stop program
        }
        else if (strncmp(opcode, "LOAD_CONST", 11) == 0) {
            next = getNext(vm);
            if (constantIsInteger(next)) {
                value.type = Int;
                value.size = 1;
                value.value.intVal = atoi(next);
            }
            else if (constantIsDouble(next)) {
                value.type = Dbl;
                value.size = 1;
                value.value.dblVal = atof(next);
            }
            push(vm, value);
        }
        else if (strncmp(opcode, "LOAD_BCONST", 12) == 0) {
            next = getNext(vm);
            value.type = Bool;
            value.size = 1;
            value.value.boolVal = strncmp(next, "true", strlen(next)) == 0 ? true : false;
            push(vm, value);
        }
        else if (strncmp(opcode, "DUP", len) == 0) {
            if (vm->sp == -1)
                return;
            value = vm->stack[vm->sp];
            push(vm, value);
        }
        else if (strncmp(opcode, "ADD", 4) == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "+");
            push(vm, rval);
        }
        else if (strncmp(opcode, "SUB", 4) == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "-");
            push(vm, rval);
        }
        else if (strncmp(opcode, "MUL", 4) == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "*");
            push(vm, rval);
        }
        else if (strncmp(opcode, "DIV", 4) == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "/");
            push(vm, rval);
        }
        else if (strncmp(opcode, "REM", 4) == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "mod");
            push(vm, rval);
        }
        else if (strncmp(opcode, "EQ", 3) == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval.type = Bool;
            rval.size = 1;
            rval.value.boolVal = isEqual(lhs, rhs);
            push(vm, rval);
        }
        else if (strncmp(opcode, "STORE", 6) == 0) {
            if (vm->sp == -1)
                return;
            value = pop(vm);
            vm->locals[++vm->lc] = value;
        }
        else if (strncmp(opcode, "LOAD", 5) == 0) {
            addr = atoi(getNext(vm));
            printf("Address: %d\n", addr);
            value = vm->locals[addr];
            push(vm, value);
        }
        else if (strncmp(opcode, "GSTORE", 7) == 0) {
            if (vm->sp == -1)
                return;
            value = pop(vm);
            vm->globals[++vm->gc] = value;
        }
        else if (strncmp(opcode, "GLOAD", 6) == 0) {
            addr = atoi(getNext(vm));
            value = vm->globals[addr];
            push(vm, value);
        }
        else if (strncmp(opcode, "JMP", 4) == 0) {
            addr = atoi(getNext(vm));
            vm->pc = addr;
        }
        else if (strncmp(opcode, "JMPT", len) == 0) {
            addr = atoi(getNext(vm));
            if (pop(vm).value.boolVal)
                vm->pc = addr;
        }
        else if (strncmp(opcode, "JMPF", len) == 0) {
            addr = atoi(getNext(vm));
            printf("addr: %d\n", addr);
            if (!pop(vm).value.boolVal)
                vm->pc = addr;
            printf("Jump to: %d(%s)\n", vm->pc, get(vm->code, addr));
        }
        else if (strncmp(opcode, "SELECT", len) == 0) {
            if (pop(vm).value.boolVal)
                next = get(vm->code, vm->pc++);
            else {
                vm->pc += 1;
                next = get(vm->code, vm->pc++);
            }
            printf("NEXT: %s\n", next);
            if (constantIsInteger(next)) {
                value.size = 1;
                value.type = Int;
                value.value.intVal = atoi(next);
            }
            else if (constantIsDouble(next)) {
                value.size = 1;
                value.type = Dbl;
                value.value.dblVal = atof(next);
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