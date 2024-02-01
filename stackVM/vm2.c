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
    char* label;
    StringVector* body;
} Function;

typedef struct {
    int length;
    Function code[256];
} SourceCode;

void displayCode(SourceCode code) {
    printf("length: %d\n", code.length);
    for (int i = 0; i < code.length; i++) {
        printf("%s => ", code.code[i].label);
        printStringVector(code.code[i].body);
        printf("\n");
    }
}

int findEntryPoint(SourceCode code) {
    for (int i = 0; i < code.length; i++) {
        if (strncmp(code.code[i].label, "_entry", 7) == 0)
            return i;
    }
    return -1;
}

typedef struct {
    StackMember* locals;
    StackMember* globals;
    SourceCode code;
    StackMember* stack;
    int pc;
    int sp;
    int fp;
    int lc;
    int gc;
} VM;

bool isInt(char* constant) {
    for (int i = 0; i < strlen(constant); i++) {
        if (!isdigit(constant[i]))
            return false;
    }
    return true;
}

bool isDouble(char* constant) {
    int dotCount = 0;
    for (int i = 0; i < strlen(constant); i++) {
        if (constant[i] == '.')
            dotCount++;
        else if (!isdigit(constant[i]))
            return false;
    }
    return dotCount == 1;
}

bool isBool(char* constant) {
    int len = strlen(constant);
    return strncmp(constant, "true", len) == 0 || strncmp(constant, "false", len) == 0;
}


VM* init(SourceCode code, int datasize) {
    VM* vm = malloc(sizeof(VM));
    vm->code = code;
    vm->pc = 0;
    vm->fp = findEntryPoint(code);
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
    return getFromSV(vm->code.code[vm->fp].body, vm->pc++);
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
            if (isInt(next)) {
                value.type = Int;
                value.size = 1;
                value.value.intVal = atoi(next);
            }
            else if (isDouble(next)) {
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
            next = getFromSV(vm->code.code[vm->fp].body, vm->pc);
            if (isInt(next)) { // overwrite the value of an existing variable
                vm->locals[atoi(next)] = value;
                vm->pc++;
            }
            else
                vm->locals[++vm->lc] = value;
        }
        else if (strncmp(opcode, "LOAD", 5) == 0) {
            addr = atoi(getNext(vm));
            value = vm->locals[addr];
            push(vm, value);
        }
        else if (strncmp(opcode, "GSTORE", 7) == 0) {
            if (vm->sp == -1)
                return;
            value = pop(vm);
            next = getFromSV(vm->code.code[vm->fp].body, vm->pc);
            if (isInt(next)) { // overwrite the value of an existing variable
                vm->globals[atoi(next)] = value;
                vm->pc++;
            }
            else
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
            printf("Jump to: %d(%s)\n", vm->pc, getFromSV(vm->code.code[vm->fp].body, addr));
        }
        else if (strncmp(opcode, "SELECT", len) == 0) {
            if (pop(vm).value.boolVal) {
                next = getFromSV(vm->code.code[vm->fp].body, vm->pc++);
                vm->pc++;
            }
            else {
                vm->pc++;
                next = getFromSV(vm->code.code[vm->fp].body, vm->pc++);
            }
            if (isInt(next)) {
                value.size = 1;
                value.type = Int;
                value.value.intVal = atoi(next);
            }
            else if (isDouble(next)) {
                value.size = 1;
                value.type = Dbl;
                value.value.dblVal = atof(next);
            }
            push(vm, value);
        }
        else if (strncmp(opcode, "CALL", len) == 0) {

        }
        else if (strncmp(opcode, "RET", len) == 0) {
            
        }
        else {
            printf("Unknown bytecode: %s\n", opcode);
            break;
        }
        display(vm);
    }
}

bool startsWith(char* in, char chr) {
    for (int i = 0; i < strlen(in); i++) {
        if (in[i] != ' ' &&  in[i] != '\t') {
            return in[i] == chr;
        }
    }
    return false;
}

SourceCode read_file() {
    SourceCode code;
    code.length = 0;
    Function func;
    FILE* fptr;
    fptr = fopen("input.txt", "r");
    StringVector* line;
    StringVector* out = createStringVector();
    const unsigned int MAX_LENGTH = 256;
    char buff[MAX_LENGTH];
    printf("Reading file...\n");
    while (fgets(buff, MAX_LENGTH, fptr)) {
        if (startsWith(buff, ';'))
            continue;
        else if (buff[strlen(buff) - 2] == ':') {
            func.label = getFromSV(split(buff, ":\n"), 0);
        }
        else if (strncmp(buff, "\n", strlen(buff)) == 0) {
            func.body = out;
            //free(out);
            out = createStringVector();
            code.code[code.length++] = func;
        }
        else {
            line = split(buff, " ");
            trimSV(line);
            out = concat(out, line);
            free(line);
        }
    }
    func.body = out;
    code.code[code.length++] = func;
    fclose(fptr);
    return code;
}

int main(int argc, char** argv) {
    SourceCode code = read_file();
    displayCode(code);
    VM* vm = init(code, 100);
    run(vm);
    destroy(vm);
    return 0;
}