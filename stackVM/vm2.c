#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringvector.h"

#define STACK_SIZE 100


typedef enum {
    HALT,
    LOAD_CONST,
    ADD,
    MUL,
    EQ,
    DUP,
    POP,
    STORE,
    GSTORE,
    LOAD,
    GLOAD,
    LOAD_IDX,
    STORE_IDX,
    GLOAD_IDX,
    GSTORE_IDX,
    JMPT,
    JMPF,
    SELECT
} opcode;

typedef struct {
    int* locals;
    int* globals;
    int* code;
    int* stack;
    int pc;
    int sp;
    int fp;
    int lc;
    int gc;
} VM;


VM* init(int* code, int datasize) {
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

int getNext(VM* vm) {
    return vm->code[vm->pc++];
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
    int opcode, value, addr, offset, lhs, rhs, argc, rval;
    while (1) {
        opcode = getNext(vm);
        switch(opcode) {
            case HALT:
                printf("Program execution complete\n");
                return; // stop program

            case DUP:
                if (vm->sp == -1)
                    return;
                value = vm->stack[vm->sp];
                push(vm, value);
                break;
            
            case LOAD_CONST:
                value = getNext(vm);
                push(vm, value);
                break;

            case ADD:
                rhs = pop(vm);
                lhs = pop(vm);
                push(vm, lhs + rhs);
                break;

            case MUL:
                rhs = pop(vm);
                lhs = pop(vm);
                push(vm, lhs * rhs);
                break;

            case EQ:
                rhs = pop(vm);
                lhs = pop(vm);
                push(vm, (lhs == rhs) ? 1 : 0);
                break;    

            case STORE:
                if (vm->sp == -1)
                    return;
                value = pop(vm);
                vm->locals[++vm->lc] = value;
                break;

            case LOAD:
                addr = getNext(vm);
                value = vm->locals[addr];
                push(vm, value);
                break;

            case GSTORE:
                if (vm->sp == -1)
                    return;
                value = pop(vm);
                vm->globals[++vm->gc] = value;
                break;

            case GLOAD:
                addr = getNext(vm);
                value = vm->globals[addr];
                push(vm, value);
                break;

            case JMPT:
                addr = getNext(vm);
                if (pop(vm))
                    vm->pc = addr;
                break;

            case JMPF:
                addr = getNext(vm);
                if (!pop(vm))
                    vm->pc = addr;
                break;

            case SELECT:
                addr = getNext(vm);
                if (pop(vm))
                    value = vm->code[vm->pc++];
                else {
                    vm->pc += 2;
                    value = vm->code[vm->pc];
                }
                push(vm, value);

            default:
                printf("Unknown bytecode\n");
                break;
        }
        display(vm);
    }
}

void read_file() {
    FILE* fptr;
    fptr = fopen("input.txt", "r");
    StringVector* line;
    StringVector* out = createStringVector();
    char** ptr;
    int index = 0;
    const unsigned int MAX_LENGTH = 256;
    char buff[MAX_LENGTH];
    printf("Reading file...\n");
    while (fgets(buff, MAX_LENGTH, fptr)) {
        line = split(buff, " ");
        trim(line);
        out = concat(out, line);
        free(line);
    }
    printStringVector(out);
    fclose(fptr);
}

int* generateCode(StringVector* contents) {
    int* code = malloc(sizeof(int) * contents->length);
    int len;
    for (int i = 0; i < contents->length; i++) {
        len = strlen(contents->strings[i]);
        if (strncmp(contents->strings[i], "LOAD_CONST", len) == 0)
            code[i] = LOAD_CONST;
        else if (strncmp(contents->strings[i], "ADD", len) == 0)
            code[i] = ADD;
        else if (strncmp(contents->strings[i], "DUP", len) == 0)
            code[i] = DUP;
        else if (strncmp(contents->strings[i], "EQ", len) == 0)
            code[i] = EQ;
        else if (strncmp(contents->strings[i], "SELECT", len) == 0)
            code[i] = SELECT;
        else if (strncmp(contents->strings[i], "HALT", len) == 0)
            code[i] = HALT;
    }
    return code;
}

int main(int argc, char** argv) {
    int code[] = {LOAD_CONST, 3, LOAD_CONST, 5, ADD, DUP, EQ, SELECT, 0, 12, HALT};
    VM* vm = init(code, 100);
    //run(vm);
    read_file();
    return 0;
}