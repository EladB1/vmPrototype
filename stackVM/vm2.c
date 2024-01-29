#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "stringvector.h"

#define STACK_SIZE 100

typedef enum {
    Addr = 1,
    Int,
    Dbl,
    Str,
    Arr,
    Bool
} Type;

typedef union memberVal {
    int intVal;
    double dblVal;
    bool boolVal;
    char* strVal;
    union memberVal* arrayVal;
} MemberValue;

typedef struct {
    Type type;
    MemberValue value;
    int size;
} StackMember;

typedef struct {
    int* locals;
    int* globals;
    StringVector* code;
    //int* stack;
    StackMember* stack;
    int pc;
    int sp;
    int fp;
    int lc;
    int gc;
} VM;

StackMember binaryOperation(StackMember lhs, StackMember rhs, char* operation) {

}

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
    StackMember value, lhs, rhs, rval;
    char* next;
    int len, addr;
    //int offset, argc, rval;
    while (1) {
        opcode = getNext(vm);
        len = strlen(opcode);
        if (strncmp(opcode, "HALT", len) == 0) {
            printf("Program execution complete\n");
            return; // stop program
        }
        else if (strncmp(opcode, "LOAD_CONST", len) == 0) {
            next = getNext(vm);
            printf("Is double: %d\n", constantIsDouble(next));
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
            //value = atoi(getNext(vm));
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
            if (lhs.type == Dbl || rhs.type == Dbl) {
                rval.type = Dbl;
                if (lhs.type == rhs.type)
                    rval.value.dblVal = lhs.value.dblVal + rhs.value.dblVal;
                else if (lhs.type == Int)
                    rval.value.dblVal = lhs.value.intVal + rhs.value.dblVal;
                else
                    rval.value.dblVal = lhs.value.dblVal + rhs.value.intVal;
            }
                
            else {
                rval.type = Int;
                rval.value.intVal = lhs.value.intVal + rhs.value.intVal;
            }
            push(vm, rval);
        }
        /*else if (strncmp(opcode, "SUB", len) == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            push(vm, lhs - rhs);
        }
        else if (strncmp(opcode, "MUL", len) == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            push(vm, lhs * rhs);
        }
        else if (strncmp(opcode, "DIV", len) == 0) {
            rhs = pop(vm);
            if (rhs == 0) {
                printf("Division by zero error\n");
                return;
            }
            lhs = pop(vm);
            push(vm, lhs / rhs);
        }
        else if (strncmp(opcode, "REM", len) == 0) {
            rhs = pop(vm);
            if (rhs == 0) {
                printf("Division by zero error\n");
                return;
            }
            lhs = pop(vm);
            push(vm, lhs % rhs);
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
        else if (strncmp(opcode, "JMP", len) == 0) {
            addr = atoi(getNext(vm));
            vm->pc = addr;
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
        }*/
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