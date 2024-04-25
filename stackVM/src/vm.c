#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include "vm.h"
#include "builtin.h"

#define ENTRYPOINT "_entry"
#define MAX_FRAMES 2048

int findLabelIndex(SourceCode* src, char* label) {
    for (int i = 0; i < src->length; i++) {
        if (strcmp(src->code[i].label, label) == 0)
            return i;
    }
    return -1;
}

VM* init(SourceCode* src) {
    VM* vm = malloc(sizeof(VM));
    vm->src = src;
    vm->fp = 0;
    vm->gp = -1;
    vm->state = success;
    vm->globals = malloc(sizeof(DataConstant) * (INT_MAX - 1));
    vm->callStack = malloc(sizeof(Frame*) * MAX_FRAMES);
    int index = findLabelIndex(src, ENTRYPOINT);
    if (index == -1) {
        fprintf(stderr, "Error: Could not find entry point function label: '%s'\n", ENTRYPOINT);
        return NULL;
    }
    vm->callStack[0] = loadFrame(src->code[index].body, src->code[index].jumpPoints, src->code[index].jmpCnt, 0, 0, NULL);
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
    printf("---\nfp: %d, gp: %d\n", vm->fp, vm->gp);
    print_array("Globals", vm->globals, vm->gp);
    printf("Call Stack:\n");
    Frame* frame;
    for (int i = 0; i <= vm->fp; i++) {
        frame = vm->callStack[i];
        printf("Frame %d\n\tsp: %d, pc: %d, returnAddr: %d\n\t", i, frame->sp, frame->pc, frame->returnAddr);
        print_array("Stack", frame->stack, frame->sp);
        printf("\t");
        print_array("Locals", frame->locals, frame->lp);
    }
}

bool isInt(char* constant) {
    int len = (int) strlen(constant);
    for (int i = 0; i < len; i++) {
        if (i == 0 && constant[i] == '-')
            continue;
        if (!isdigit(constant[i]))
            return false;
    }
    return true;
}

bool isDouble(char* constant) {
    int dotCount = 0;
    int len = (int) strlen(constant);
    for (int i = 0; i < len; i++) {
        if (i == 0 && constant[i] == '-')
            continue;
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

char* removeQuotes(char* in) {
    int len = strlen(in);
    char out[len - 2];
    int index = 0;
    for (int i = 1; i < len - 1; i++) {
        out[index++] = in[i];
    }
    out[index] = '\0';
    return strdup(out);
}

void stepOver(VM* vm) {
    incrementPC(vm->callStack[vm->fp]);
}

void jump(VM* vm, char* label, char** enterJump, int* jumpedFrom) {
    int addr = getJumpStart(vm->callStack[vm->fp], label);
    if (addr == -1) {
        fprintf(stderr, "Error: Could not find jump point '%s'\n", label);
        vm->state = unknown_bytecode;
    }
    if (strcmp(*enterJump, label) != 0) // only change jumpedFrom if enterJump changes
        *jumpedFrom = vm->callStack[vm->fp]->pc + 1;
    *enterJump = label;
    setPC(vm->callStack[vm->fp], addr);
}

void skipJump(VM* vm, char* label) {
    int addr = getJumpEnd(vm->callStack[vm->fp], label);
    if (addr == -1) {
        fprintf(stderr, "Error: Could not find jump point '%s'\n", label);
        vm->state = unknown_bytecode;
    }
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

void handleArrayReturn(DataConstant* returnValue, DataConstant* source, Frame* target) {
    returnValue->value.address = ++(target->lp) + target->locals;
    for (int i = 0; i < returnValue->size; i++) {
        target->locals[target->lp++] = source[i];
    }
    if (returnValue->size > 0)
        target->lp--;
}

void convertLocalArrayToGlobal(VM* vm, DataConstant* array) {
    Frame* currentFrame = vm->callStack[vm->fp];
    DataConstant* start = array->value.address;
    DataConstant* stop = start + array->size;
    array->value.address = ++(vm->gp) + vm->globals;
    for (DataConstant* curr = start; curr != stop; curr++) {
        vm->globals[vm->gp++] = *curr;
    }
    if (array->size > 0) {
        vm->gp--;
        int startIndex = start - currentFrame->locals;
        memmove(&currentFrame->locals[startIndex], &currentFrame->locals[startIndex + array->size], sizeof(DataConstant) * (currentFrame->lp + 1 - array->size - 1));
        currentFrame->lp -= array->size;
    }
}

ExitCode run(VM* vm, bool verbose) {
    if (verbose)
        printf("Running program...\n");
    char* opcode;
    DataConstant value, lhs, rhs, rval;
    Frame* currentFrame;
    char* next;
    int addr, argc, offset;
    char* enterJump = "";
    int jumpedFrom = 0;
    JumpPoint jumpPoint;
    bool skipped = false;
    while (1) {
        if (vm->state != success)
            return vm->state;
        opcode = getNext(vm);
        currentFrame = vm->callStack[vm->fp];
        for (int i = 0; i < currentFrame->jc; i++) {
            jumpPoint = currentFrame->jumps[i];
            if (currentFrame->pc - 1 == jumpPoint.start) {
                if (strlen(enterJump) == 0) {
                    skipJump(vm, jumpPoint.label);
                    skipped = true;
                }
                else {
                    if (strcmp(jumpPoint.label, enterJump) == 0) {
                        enterJump = "";
                        break;
                    }
                    skipJump(vm, jumpPoint.label);
                    skipped = true;
                }
            }
        }
        if (skipped) {
            skipped = false;
            continue;
        }
        if (strcmp(opcode, "EJMP") == 0) {
            if (jumpedFrom != 0) {
                setPC(vm->callStack[vm->fp], jumpedFrom - 1);
                jumpedFrom = 0;
            }
            continue;
        }
        if (strcmp(opcode, "HALT") == 0) {
            if (verbose)
                printf("-----\nProgram execution complete\n");
            return success; // stop program with a successful exit code
        }
        else if (strcmp(opcode, "LOAD_CONST") == 0) {
            next = getNext(vm);
            if (isInt(next))
                value = readInt(next);
            else if (isDouble(next))
                value = readDouble(next);
            else if (isBool(next))
                value = readBoolean(next);
            else if (startsWith(next, '"')) {
                value = createString(removeQuotes(next));
            }
            else if (strcmp(next, "NULL") == 0)
                value = createNull();
            else if (strcmp(next, "NONE") == 0)
                value = createNone();
            push(vm, value);
        }
        else if (strcmp(opcode, "DUP") == 0) {
            if (stackIsEmpty(currentFrame)) {
                fprintf(stderr, "Error: No value to duplicate\n");
                return operation_err;
            }
            value = top(vm);
            push(vm, value);
        }
        else if (strcmp(opcode, "POP") == 0) {
            if (stackIsEmpty(currentFrame)) {
                fprintf(stderr, "Error: Attempted to POP empty stack\n");
                return operation_err;
            }
            pop(vm);
        }
        else if (strcmp(opcode, "CONCAT") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            if (lhs.type == Str) {
                size_t size = strlen(lhs.value.strVal) + strlen(rhs.value.strVal) + 1;
                char* concat = strncat(strdup(lhs.value.strVal), rhs.value.strVal, size); // using strdup here prevents the LHS value from being overwritten in the heap
                rval = createString(concat);
            }
            else if (lhs.type == Addr) {
                rval.type = Addr;
                rval.size = lhs.size + rhs.size;
                rval.length = lhs.length + rhs.length;
                rval.value.intVal = ++vm->gp;
                for (int i = 0; i < lhs.length; i++) {
                    vm->globals[vm->gp] = vm->globals[lhs.value.intVal + i];
                    vm->gp++;
                }
                for (int i = 0; i < rhs.length; i++) {
                    vm->globals[vm->gp] = vm->globals[rhs.value.intVal + i];
                    if (i < rhs.length - 1)
                        vm->gp++;
                }
                if (rval.size > rval.length) {
                    vm->gp++;
                    for (int i = rval.length; i < rval.size; i++) {
                        vm->globals[vm->gp] = createNone();
                        if (i < rval.size - 1)
                            vm->gp++;
                    }
                }
            }
            push(vm, rval);
        }
        else if (strcmp(opcode, "REPEATSTR") == 0) {
            rhs = pop(vm);
            argc = atoi(getNext(vm));
            if (argc <= 0)
                push(vm, createString(""));
            else if (argc == 1)
                push(vm, rhs);
            else {
                strcpy(next, rhs.value.strVal);
                for (int i = 1; i < argc; i++) {
                    next = strncat(next, rhs.value.strVal, strlen(rhs.value.strVal));
                }
                push(vm, createString(next));
            }
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
            if (rval.type == None)
                return operation_err;
            push(vm, rval);
        }
        else if (strcmp(opcode, "REM") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "mod");
            if (rval.type == None)
                return operation_err;
            push(vm, rval);
        }
        else if (strcmp(opcode, "POW") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "exp");
            if (rval.type == None)
                return operation_err;
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
        else if (strcmp(opcode, "STORE") == 0) {
            if (stackIsEmpty(currentFrame)) {
                fprintf(stderr, "Error: no value to store\n");
                return operation_err;
            }
            storeValue(vm);
        }
        else if (strcmp(opcode, "LOAD") == 0) {
            value = load(vm);
            push(vm, value);
        }
        else if (strcmp(opcode, "GSTORE") == 0) {
            if (stackIsEmpty(currentFrame)) {
                fprintf(stderr, "Error: no value to store\n");
                return operation_err;
            }
            value = pop(vm);
            if (value.type == Addr)
                convertLocalArrayToGlobal(vm, &value);
            next = peekNext(vm);
            if (isInt(next)) { // overwrite the value of an existing variable
                vm->globals[atoi(next)] = value;
                stepOver(vm);
            }
            else
                vm->globals[++vm->gp] = value;
        }
        else if (strcmp(opcode, "GLOAD") == 0) {
            addr = atoi(getNext(vm));
            value = vm->globals[addr];
            push(vm, value);
        }
        else if (strcmp(opcode, "JMP") == 0) {
            next = getNext(vm);
            jump(vm, next, &enterJump, &jumpedFrom);
        }
        else if (strcmp(opcode, "JMPT") == 0) {
            next = getNext(vm);
            if (pop(vm).value.boolVal) {
                jumpedFrom = currentFrame->pc + 1;
                jump(vm, next, &enterJump, &jumpedFrom);
            }
        }
        else if (strcmp(opcode, "JMPF") == 0) {
            next = getNext(vm);
            if (!pop(vm).value.boolVal)
                jump(vm, next, &enterJump, &jumpedFrom);
        }
        else if (strcmp(opcode, "SJMPT") == 0) {
            // short circuit for and/or statements
            next = getNext(vm);
            if (top(vm).value.boolVal)
                jump(vm, next, &enterJump, &jumpedFrom);
        }
        else if (strcmp(opcode, "SJMPF") == 0) {
            // short circuit for and/or statements
            next = getNext(vm);
            if (!top(vm).value.boolVal)
                jump(vm, next, &enterJump, &jumpedFrom);
        }
        else if (strcmp(opcode, "EJMPT") == 0) {
            // skip the rest of the jump block
            if (pop(vm).value.boolVal) {
                next = getNext(vm);
                while (strcmp(next, "EJMP") != 0) {
                    next = getNext(vm);
                }
                if (jumpedFrom != 0) {
                    setPC(vm->callStack[vm->fp], jumpedFrom - 1);
                    jumpedFrom = 0;
                }
                continue;
            }
        }
        else if (strcmp(opcode, "EJMPF") == 0) {
            // skip the rest of the jump block
            if (!pop(vm).value.boolVal) {
                next = getNext(vm);
                while (strcmp(next, "EJMP") != 0) {
                    next = getNext(vm);
                }
                if (jumpedFrom != 0) {
                    setPC(vm->callStack[vm->fp], jumpedFrom - 1);
                    jumpedFrom = 0;
                }
                continue;
            }
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
                value = readInt(next);
            else if (isDouble(next))
                value = readDouble(next);
            else if (isBool(next))
                value = readBoolean(next);
            else if (startsWith(next, '"')) {
                value = createString(removeQuotes(next));
            }
            else if (strcmp(next, "NULL") == 0)
                value = createNull();
            else if (strcmp(next, "NONE") == 0)
                value = createNone();
            push(vm, value);
        }
        else if (strcmp(opcode, "CALL") == 0) {
            next = getNext(vm);
            argc = atoi(getNext(vm));
            DataConstant params[argc];
            for (int i = 0; i < argc; i++) {
                params[i] = pop(vm);
            }
            if (isBuiltinFunction(next)) {
                rval = callBuiltinFunction(next, argc, params, &vm->gp, &vm->globals, &(vm->state));
                if (vm->state != success)
                    return vm->state;
                if (rval.type != None)
                    push(vm, rval);
            }
            else {
                addr = findLabelIndex(vm->src, next);
                if (addr == -1) {
                    fprintf(stderr, "Error: could not find function '%s'\n", next);
                    return unknown_bytecode;
                }
                Frame* frame = loadFrame(vm->src->code[addr].body, vm->src->code[addr].jumpPoints, vm->src->code[addr].jmpCnt, currentFrame->pc, argc, params);
                vm->callStack[++vm->fp] = frame;
            }
            
        }
        else if (strcmp(opcode, "RET") == 0) {
            rval = pop(vm);
            addr = currentFrame->returnAddr;
            Frame* caller = vm->callStack[--vm->fp];
            setPC(caller, addr);
            if (rval.type != None) {
                if (rval.type == Addr)
                    handleArrayReturn(&rval, currentFrame->locals, caller);
                push(vm, rval);
            }
            deleteFrame(currentFrame);
        }
        else if (strcmp(opcode, "BUILDARR") == 0) {
            int capacity = atoi(getNext(vm));
            if (isInt(peekNext(vm)))
                argc = atoi(getNext(vm));
            else {
                argc = capacity;
                capacity = top(vm).value.intVal;
            }
            if (argc > capacity) {
                fprintf(stderr, "Error: Attempted to build array of length %d which exceeds capacity %d\n", argc, capacity);
                return memory_err;
            }
            rval = createAddr(++currentFrame->lp + currentFrame->locals, capacity, argc);
            for (int i = 0; i < argc; i++) {
                currentFrame->locals[currentFrame->lp] = pop(vm);
                if (i < argc - 1)
                    currentFrame->lp++;
            }
            if (capacity > argc) {
                if (argc != 0)
                    currentFrame->lp++;
                for (int i = argc; i < capacity; i++) {
                    currentFrame->locals[currentFrame->lp] = createNone();
                    if (i < capacity - 1)
                        currentFrame->lp++;
                }
            }
            push(vm, rval);
        }
        else if (strcmp(opcode, "COPYARR") == 0) {
            rhs = pop(vm);
            rval = copyAddr(rhs, &vm->gp, &vm->globals);
            push(vm, rval);
        }
        else if (strcmp(opcode, "AGET") == 0) {
            offset = pop(vm).value.intVal;
            lhs = pop(vm);
            addr = lhs.value.intVal;
            if (offset > lhs.size || offset < 0) {
                fprintf(stderr, "Error: Array index %d out of range %d\n", offset, lhs.size);
                return memory_err;
            }
            rval = vm->globals[addr + offset];
            push(vm, rval);
        }
        else if (strcmp(opcode, "ASTORE") == 0) {
            offset = pop(vm).value.intVal;
            lhs = pop(vm);
            addr = lhs.value.intVal;
            if (offset >= lhs.size || offset < 0) {
                fprintf(stderr, "Error: Array index %d out of range %d\n", offset, lhs.size);
                return memory_err;
            }
            rhs = pop(vm);
            rval = vm->globals[addr + offset];
            if (rval.type == None) {
                if (offset > lhs.length + 1) {
                    fprintf(stderr, "Error: Cannot write to index %d since previous index values are not initialized\n", offset);
                    return memory_err;
                }
                lhs.length++;
            }
            vm->globals[addr + offset] = rhs;
            push(vm, lhs);
        }
        else {
            fprintf(stderr, "Unknown bytecode: '%s'\n", opcode);
            return unknown_bytecode;
        }
        if (verbose)
            display(vm);
    }
    return success;
}