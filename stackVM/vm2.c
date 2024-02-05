#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>

#include "filereader.h"
#include "frame.h"

#define STACK_SIZE 100
#define ENTRYPOINT "_entry"

int findLabelIndex(SourceCode src, char* label) {
    for (int i = 0; i < src.length; i++) {
        if (strcmp(src.code[i].label, label) == 0)
            return i;
    }
    return -1;
}

typedef struct {
    DataConstant* globals;
    SourceCode src;
    Frame** callStack;
    int fp;
    int gc;
} VM;

VM* init(SourceCode src, int datasize) {
    VM* vm = malloc(sizeof(VM));
    vm->src = src;
    vm->fp = 0;
    vm->gc = -1;
    vm->globals = malloc(sizeof(int) * datasize);
    vm->callStack = malloc(sizeof(Frame*) * src.length);
    int index = findLabelIndex(src, ENTRYPOINT);
    vm->callStack[vm->fp] = loadFrame(src.code[index].body, 0, index, 0, NULL);
    return vm;
}

void destroy(VM* vm) {
    free(vm->globals);
    free(vm->callStack);
    free(vm);
}

void push(VM* vm, DataConstant value) {
    framePush(vm->callStack[vm->fp], value);
}

DataConstant pop(VM* vm) {
    return framePop(vm->callStack[vm->fp]);
}

DataConstant top(VM* vm) {
    return frameTop(vm->callStack[vm->fp]);
}

char* getNext(VM* vm) {
    return getNextInstruction(vm->callStack[vm->fp]);
}

char* peekNext(VM* vm) {
    return peekNextInstruction(vm->callStack[vm->fp]);
}

void display(VM* vm) {
    printf("---\nfp: %d\n", vm->fp);
    print_array("Globals", vm->globals, vm->gc);
    printf("Call Stack:\n");
    Frame* frame;
    for (int i = 0; i <= vm->fp; i++) {
        frame = vm->callStack[i];
        printf("Frame %d\n\tsp: %d, pc: %d, returnAddr: %d, frameAddr: %d\n\t", i, frame->sp, frame->pc, frame->returnAddr, frame->frameAddr);
        print_array("Stack", frame->stack, frame->sp);
        printf("\t");
        print_array("Locals", frame->locals, frame->lc);
    }
}

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
    return strcmp(constant, "true") == 0 || strcmp(constant, "false") == 0;
}

void stepOver(VM* vm) {
    incrementPC(vm->callStack[vm->fp]);
}

void jump(VM* vm, int addr) {
    setPC(vm->callStack[vm->fp], addr);
}

DataConstant load(VM* vm) {
    int addr = atoi(getNext(vm));
    return loadLocal(vm->callStack[vm->fp], addr);
}

void storeValue(VM* vm) {
    DataConstant value = pop(vm);
    char* next = peekNext(vm);
    if (isInt(next)) {
        storeLocalAtAddr(vm->callStack[vm->fp], value, atoi(next));
        stepOver(vm);
    }
    else {
        storeLocal(vm->callStack[vm->fp], value);
    }
}

void run(VM* vm) {
    printf("Running program...\n");
    char* opcode;
    DataConstant value, lhs, rhs, rval;
    Frame* currentFrame;
    char* next;
    int addr, argc;
    while (1) {
        opcode = getNext(vm);
        currentFrame = vm->callStack[vm->fp];
        if (strcmp(opcode, "HALT") == 0) {
            printf("-----\nProgram execution complete\n");
            return; // stop program
        }
        else if (strcmp(opcode, "LOAD_CONST") == 0) {
            next = getNext(vm);
            if (isInt(next))
                value = createInt(next);
            else if (isDouble(next))
                value = createDouble(next);
            else if (isBool(next))
                value = createBoolean(next);
            else if (startsWith(next, '"')) {
                value = createString(next);
            }
            else if (strcmp(next, "NULL") == 0)
                value = createNull();
            else if (strcmp(next, "NONE") == 0)
                value = createNone();
            push(vm, value);
        }
        else if (strcmp(opcode, "DUP") == 0) {
            if (stackIsEmpty(currentFrame))
                return;
            value = top(vm);
            push(vm, value);
        }
        else if (strcmp(opcode, "ADD") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "+");
            push(vm, rval);
        }
        else if (strcmp(opcode, "SUB") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "-");
            push(vm, rval);
        }
        else if (strcmp(opcode, "MUL") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "*");
            push(vm, rval);
        }
        else if (strcmp(opcode, "DIV") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "/");
            push(vm, rval);
        }
        else if (strcmp(opcode, "REM") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "mod");
            push(vm, rval);
        }
        else if (strcmp(opcode, "POW") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "exp");
            push(vm, rval);
        }
        else if (strcmp(opcode, "EQ") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, "==");
            push(vm, rval);
        }
        else if (strcmp(opcode, "NE") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, "!=");
            push(vm, rval);
        }
        else if (strcmp(opcode, "LT") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, "<");
            push(vm, rval);
        }
        else if (strcmp(opcode, "LE") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, "<=");
            push(vm, rval);
        }
        else if (strcmp(opcode, "GT") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, ">");
            push(vm, rval);
        }
        else if (strcmp(opcode, "GE") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, ">=");
            push(vm, rval);
        }
        else if (strcmp(opcode, "NOT") == 0) {
            rhs = pop(vm);
            rval.type = Bool;
            rval.size = 1;
            rval.value.boolVal = !rhs.value.boolVal;
            push(vm, rval);
        }
        else if (strcmp(opcode, "OR") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval.type = Bool;
            rval.size = 1;
            rval.value.boolVal = lhs.value.boolVal || rhs.value.boolVal;
            push(vm, rval);
        }
        else if (strcmp(opcode, "AND") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval.type = Bool;
            rval.size = 1;
            rval.value.boolVal = lhs.value.boolVal && rhs.value.boolVal;
            push(vm, rval);
        }
        else if (strcmp(opcode, "XOR") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval.type = Int;
            rval.size = 1;
            if (lhs.type == Int)
                rval.value.intVal = rhs.type == Int ? lhs.value.intVal ^ rhs.value.intVal : lhs.value.intVal ^ rhs.value.boolVal;
            if (lhs.type == Bool)
                rval.value.intVal = rhs.type == Bool ? lhs.value.boolVal ^ rhs.value.boolVal : lhs.value.boolVal ^ rhs.value.intVal;
            push(vm, rval);
        }
        else if (strcmp(opcode, "B_AND") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval.type = Int;
            rval.size = 1;
            if (lhs.type == Int)
                rval.value.intVal = rhs.type == Int ? lhs.value.intVal & rhs.value.intVal : lhs.value.intVal & rhs.value.boolVal;
            if (lhs.type == Bool)
                rval.value.intVal = rhs.type == Bool ? lhs.value.boolVal & rhs.value.boolVal : lhs.value.boolVal & rhs.value.intVal;
            push(vm, rval);
        }
        else if (strcmp(opcode, "WAIT") == 0) {
            usleep(1000000 * atof(getNext(vm)));
        }
        else if (strcmp(opcode, "STORE") == 0) {
            if (stackIsEmpty(currentFrame))
                return;
            storeValue(vm);
        }
        else if (strcmp(opcode, "LOAD") == 0) {
            value = load(vm);
            push(vm, value);
        }
        else if (strcmp(opcode, "GSTORE") == 0) {
            if (stackIsEmpty(currentFrame))
                return;
            value = pop(vm);
            next = peekNext(vm);
            if (isInt(next)) { // overwrite the value of an existing variable
                vm->globals[atoi(next)] = value;
                stepOver(vm);
            }
            else
                vm->globals[++vm->gc] = value;
        }
        else if (strcmp(opcode, "GLOAD") == 0) {
            addr = atoi(getNext(vm));
            value = vm->globals[addr];
            push(vm, value);
        }
        else if (strcmp(opcode, "JMP") == 0) {
            addr = atoi(getNext(vm));
            jump(vm, addr);
        }
        else if (strcmp(opcode, "JMPT") == 0) {
            addr = atoi(getNext(vm));
            if (pop(vm).value.boolVal)
                jump(vm, addr);
        }
        else if (strcmp(opcode, "JMPF") == 0) {
            addr = atoi(getNext(vm));
            if (!pop(vm).value.boolVal)
                jump(vm, addr);
        }
        else if (strcmp(opcode, "SJMPT") == 0) {
            // short circuit for and/or statements
            addr = atoi(getNext(vm));
            if (top(vm).value.boolVal)
                jump(vm, addr);
        }
        else if (strcmp(opcode, "SJMPF") == 0) {
            // short circuit for and/or statements
            addr = atoi(getNext(vm));
            if (!top(vm).value.boolVal)
                jump(vm, addr);
        }
        else if (strcmp(opcode, "SELECT") == 0) {
            if (pop(vm).value.boolVal) {
                next = getNext(vm);
                stepOver(vm);
            }
            else {
                stepOver(vm);
                next = getNext(vm);
            }
            if (isInt(next))
                value = createInt(next);
            else if (isDouble(next))
                value = createDouble(next);
            else if (isBool(next))
                value = createBoolean(next);
            push(vm, value);
        }
        else if (strcmp(opcode, "CALL") == 0) {
            next = getNext(vm);
            argc = atoi(getNext(vm));
            DataConstant params[argc];
            for (int i = 0; i < argc; i++) {
                params[i] = pop(vm);
            }
            addr = findLabelIndex(vm->src, next);
            Frame* frame = loadFrame(vm->src.code[addr].body, currentFrame->pc, vm->fp, argc, params);
            vm->callStack[++vm->fp] = frame;
        }
        else if (strcmp(opcode, "RET") == 0) {
            rval = pop(vm);
            addr = currentFrame->returnAddr;
            Frame* caller = vm->callStack[--vm->fp];
            setPC(caller, addr);
            deleteFrame(vm->callStack[vm->fp + 1]);
            push(vm, rval);
        }
        else {
            printf("Unknown bytecode: %s\n", opcode);
            break;
        }
        display(vm);
    }
}

int main(int argc, char** argv) {
    SourceCode src = read_file("input.txt");
    displayCode(src);
    VM* vm = init(src, 100);
    run(vm);
    destroy(vm);
    return 0;
}