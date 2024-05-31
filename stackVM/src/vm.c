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

int findLabelIndex(SourceCode* src, char* label) {
    for (int i = 0; i < src->length; i++) {
        if (strcmp(src->code[i].label, label) == 0)
            return i;
    }
    return -1;
}

VM* init(SourceCode* src, VMConfig conf) {
    VM* vm = malloc(sizeof(VM));
    vm->src = src;
    vm->fp = 0;
    vm->gp = -1;
    vm->state = success;

    vm->framesSoftMax = conf.dynamicResourceExpansionEnabled ? conf.framesSoftMax : conf.framesHardMax;
    vm->framesHardMax = conf.framesHardMax;

    vm->globalsHardMax = conf.globalsHardMax / sizeof(DataConstant);
    vm->globalsSoftMax = conf.dynamicResourceExpansionEnabled ? conf.globalsSoftMax / sizeof(DataConstant) : vm->globalsHardMax;

    vm->localsHardMax = conf.localsHardMax / sizeof(DataConstant);
    vm->localsSoftMax = conf.dynamicResourceExpansionEnabled ? conf.localsSoftMax / sizeof(DataConstant) : vm->localsHardMax;

    vm->stackHardMax = conf.stackSizeHardMax / sizeof(DataConstant);
    vm->stackSoftMax = conf.dynamicResourceExpansionEnabled ? conf.stackSizeSoftMax / sizeof(DataConstant) : vm->stackHardMax;

    vm->globals = malloc(conf.dynamicResourceExpansionEnabled || conf.globalsSoftMax == conf.globalsHardMax ? conf.globalsSoftMax : conf.globalsHardMax);
    vm->callStack = malloc(conf.dynamicResourceExpansionEnabled || conf.framesSoftMax == conf.framesHardMax ? conf.framesSoftMax : conf.framesHardMax);
    vm->useHeapStorageBackup = conf.useHeapStorageBackup;
    int index = findLabelIndex(src, ENTRYPOINT);
    if (index == -1) {
        fprintf(stderr, "Error: Could not find entry point function label: '%s'\n", ENTRYPOINT);
        return NULL;
    }
    vm->callStack[0] = loadFrame(src->code[index].body, src->code[index].jumpPoints, src->code[index].jmpCnt, vm->stackSoftMax, vm->localsSoftMax, 0, 0, NULL);
    return vm;
}

void destroy(VM* vm) {
    free(vm->globals);
    free(vm->callStack);
    free(vm);
}

void push(VM* vm, DataConstant value, bool verbose) {
    Frame* frame = vm->callStack[vm->fp];
    int end = frame->sp + 1;
    if (!frame->expandedStack && vm->stackSoftMax != vm->stackHardMax && end >= vm->stackSoftMax - 1 && end < vm->stackHardMax) {
        frame->expandedStack = true;
        if (verbose)
            printf("INFO: Expanding current frame stack from %ld to %ld\n", vm->stackSoftMax, vm->stackHardMax);
        frame = expandStack(frame, vm->stackHardMax);
    }
    if (end > vm->stackHardMax) {
        fprintf(stderr, "StackOverflow: Exceeded stack space of %ld\n", vm->stackHardMax);
        vm->state = memory_err;
        return;
    }
    framePush(frame, value);
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

void storeValue(VM* vm, bool verbose) {
    DataConstant value = pop(vm);
    Frame* frame = vm->callStack[vm->fp];
    char* next = peekNext(vm);
    if (isInt(next)) {
        storeLocalAtAddr(frame, value, atoi(next));
        stepOver(vm);
    }
    else {
        int total = frame->lp + value.size + 1;
        if (!frame->expandedLocals && vm->localsSoftMax != vm->localsHardMax && total >= vm->localsSoftMax - 1 && total < vm->localsHardMax) {
            if (verbose)
                printf("INFO: Expanding local storage from %ld to %ld\n", vm->localsSoftMax, vm->localsHardMax);
            *frame = *(expandLocals(frame, vm->localsHardMax));
            frame->expandedLocals = true;
        }
        if (total > vm->localsHardMax) {
            fprintf(stderr, "StackOverflow: Exceeded local storage maximum of %ld\n", vm->localsHardMax);
            vm->state = memory_err;
            return;
        }
        storeLocal(frame, value);
    }
}

ArrayTarget checkAndRetrieveArrayValuesTarget(VM* vm, Frame* frame, int arraySize, bool* globalsExpanded, bool verbose) {
    ArrayTarget arrayTarget;
    arrayTarget.target = frame->locals;
    arrayTarget.targetp = &frame->lp;
    arrayTarget.frame = frame;
    int total = frame->lp + arraySize + 1;
    if (!frame->expandedLocals && vm->localsSoftMax != vm->localsHardMax && total >= vm->localsSoftMax - 1 && total < vm->localsHardMax) {
        if (verbose)
            printf("INFO: Expanding local storage from %ld to %ld\n", vm->localsSoftMax, vm->localsHardMax);
        arrayTarget.frame = expandLocals(frame, vm->localsHardMax);
        arrayTarget.frame->expandedLocals = true;
        arrayTarget.target = frame->locals;
        arrayTarget.targetp = &frame->lp;
    }
    if (total > vm->localsHardMax) {
        if (vm->useHeapStorageBackup) {
            arrayTarget.target = vm->globals;
            arrayTarget.targetp = &vm->gp;
            total = vm->gp + arraySize + 1;
            if (!(*globalsExpanded) && vm->globalsSoftMax != vm->globalsHardMax && total >= vm->globalsSoftMax - 1 && total < vm->globalsHardMax) {
                if (verbose)
                    printf("INFO: Expanding size of globals from %ld to %ld\n", vm->globalsSoftMax, vm->globalsHardMax);
                *globalsExpanded = true;
                vm->globals = realloc(vm->globals, sizeof(DataConstant) * vm->globalsHardMax);
                arrayTarget.target = vm->globals;
            }
            if (total > vm->globalsHardMax) {
                fprintf(stderr, "HeapOverflow: Exceeded local storage maximum of %ld and global storage maximum of %ld\n", vm->localsHardMax, vm->globalsHardMax);
                vm->state = memory_err;
                return arrayTarget;
            }
            if (verbose)
                printf("INFO: Using heap storage for array %p\n", vm->globals + vm->gp + 1);
        }
        else {
            fprintf(stderr, "StackOverflow: Exceeded local storage maximum of %ld\n", vm->localsHardMax);
            vm->state = memory_err;
            return arrayTarget;
        }
    }
    return arrayTarget;
}

ExitCode run(VM* vm, bool verbose) {
    if (verbose)
        printf("Running program...\n");
    char* opcode;
    DataConstant value, lhs, rhs, rval;
    Frame* currentFrame;
    char* next;
    int addr, argc, offset, total;
    char* enterJump = "";
    int jumpedFrom = 0;
    JumpPoint jumpPoint;
    bool skipped = false;
    bool framesExpanded = false;
    bool globalsExpanded = false;
    ArrayTarget arrayTarget;
    while (1) {
        if (vm->state != success)
            return vm->state;
        if (verbose)
            display(vm);
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
            push(vm, value, verbose);
        }
        else if (strcmp(opcode, "DUP") == 0) {
            if (stackIsEmpty(currentFrame)) {
                fprintf(stderr, "Error: No value to duplicate\n");
                return operation_err;
            }
            value = top(vm);
            push(vm, value, verbose);
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
                arrayTarget = checkAndRetrieveArrayValuesTarget(vm, currentFrame, lhs.size + rhs.size, &globalsExpanded, verbose);
                if (vm->state != success)
                    return vm->state;
                *currentFrame = *arrayTarget.frame;
                if (lhs.value.address != vm->globals)
                    lhs.value.address = currentFrame->locals;
                if (rhs.value.address != vm->globals)
                    rhs.value.address = currentFrame->locals;
                rval = createAddr(arrayTarget.target, *(arrayTarget.targetp) + 1, lhs.size + rhs.size, lhs.length + rhs.length);
                DataConstant* start = getArrayStart(lhs);
                DataConstant* stop = start + lhs.length;
                for (DataConstant* curr = start; curr != stop; curr++) {
                    arrayTarget.target[++(*arrayTarget.targetp)] = *curr;
                }
                start = getArrayStart(rhs);
                stop = start + rhs.length;
                for (DataConstant* curr = start; curr != stop; curr++) {
                    arrayTarget.target[++(*arrayTarget.targetp)] = *curr;
                }
                if (rval.length < rval.size) {
                    for (int i = rval.length; i < rval.size; i++) {
                        arrayTarget.target[++(*arrayTarget.targetp)] = createNone();
                    }
                }
            }
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "REPEATSTR") == 0) {
            rhs = pop(vm);
            argc = atoi(getNext(vm));
            if (argc <= 0)
                push(vm, createString(""), verbose);
            else if (argc == 1)
                push(vm, rhs, verbose);
            else {
                strcpy(next, rhs.value.strVal);
                for (int i = 1; i < argc; i++) {
                    next = strncat(next, rhs.value.strVal, strlen(rhs.value.strVal));
                }
                push(vm, createString(next), verbose);
            }
        }
        else if (strcmp(opcode, "ADD") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "+");
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "SUB") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "-");
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "MUL") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "*");
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "DIV") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "/");
            if (rval.type == None)
                return operation_err;
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "REM") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "mod");
            if (rval.type == None)
                return operation_err;
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "POW") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = binaryArithmeticOperation(lhs, rhs, "exp");
            if (rval.type == None)
                return operation_err;
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "EQ") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, "==");
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "NE") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, "!=");
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "LT") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, "<");
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "LE") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, "<=");
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "GT") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, ">");
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "GE") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval = compareData(lhs, rhs, ">=");
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "NOT") == 0) {
            rhs = pop(vm);
            rval.type = Bool;
            rval.size = 1;
            rval.value.boolVal = !rhs.value.boolVal;
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "OR") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval.type = Bool;
            rval.size = 1;
            rval.value.boolVal = lhs.value.boolVal || rhs.value.boolVal;
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "AND") == 0) {
            rhs = pop(vm);
            lhs = pop(vm);
            rval.type = Bool;
            rval.size = 1;
            rval.value.boolVal = lhs.value.boolVal && rhs.value.boolVal;
            push(vm, rval, verbose);
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
            push(vm, rval, verbose);
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
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "STORE") == 0) {
            if (stackIsEmpty(currentFrame)) {
                fprintf(stderr, "Error: no value to store\n");
                return operation_err;
            }
            storeValue(vm, verbose);
        }
        else if (strcmp(opcode, "LOAD") == 0) {
            value = load(vm);
            push(vm, value, verbose);
        }
        else if (strcmp(opcode, "GSTORE") == 0) {
            if (stackIsEmpty(currentFrame)) {
                fprintf(stderr, "Error: no value to store\n");
                return operation_err;
            }
            value = pop(vm);
            total = vm->gp + value.size + 1;
            if (value.type == Addr) {
                //printf("%ld\n", vm->globalsHardMax);
                if (!globalsExpanded && vm->globalsSoftMax != vm->globalsHardMax && total >= vm->globalsSoftMax - 1 && total < vm->globalsHardMax) {
                    if (verbose)
                        printf("INFO: Expanding size of globals from %ld to %ld\n", vm->globalsSoftMax, vm->globalsHardMax);
                    globalsExpanded = true;
                    vm->globals = realloc(vm->globals, sizeof(DataConstant) * vm->globalsHardMax);
                }
                if (total > vm->globalsHardMax) {
                    fprintf(stderr, "HeapOverflow: Global storage hard maximum of %ld reached\n", vm->globalsHardMax);
                    return memory_err;
                }
                value = copyAddr(value, &vm->gp, &vm->globals);
            }
            next = peekNext(vm);
            if (isInt(next)) { // overwrite the value of an existing variable
                vm->globals[atoi(next)] = value;
                stepOver(vm);
            }
            else {
                if (!globalsExpanded && vm->globalsSoftMax != vm->globalsHardMax && total >= vm->globalsSoftMax - 1 && total < vm->globalsHardMax) {
                    if (verbose)
                        printf("INFO: Expanding size of globals from %ld to %ld\n", vm->globalsSoftMax, vm->globalsHardMax);
                    globalsExpanded = true;
                    vm->globals = realloc(vm->globals, sizeof(DataConstant) * vm->globalsHardMax);
                }
                if (total > vm->globalsHardMax) {
                    fprintf(stderr, "HeapOverflow: Global storage hard maximum of %ld reached\n", vm->globalsHardMax);
                    return memory_err;
                }
                vm->globals[++vm->gp] = value;
            }
        }
        else if (strcmp(opcode, "GLOAD") == 0) {
            addr = atoi(getNext(vm));
            value = vm->globals[addr];
            push(vm, value, verbose);
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
                value = pop(vm); // save the first value to push it back onto the stack
                pop(vm); // pop the second value off the stack
                push(vm, value, verbose);
            }
            else {
                pop(vm); // use the second value on the stack as the "return" value
            }
        }
        else if (strcmp(opcode, "CALL") == 0) {
            next = getNext(vm);
            argc = atoi(getNext(vm));
            DataConstant params[argc];
            for (int i = 0; i < argc; i++) {
                params[i] = pop(vm);
            }
            if (isBuiltinFunction(next)) {
                rval = callBuiltinFunction(next, argc, params, vm, currentFrame, &globalsExpanded, verbose);
                if (vm->state != success)
                    return vm->state;
                if (rval.type != None) {
                    push(vm, rval, verbose);
                }
            }
            else {
                addr = findLabelIndex(vm->src, next);
                if (addr == -1) {
                    fprintf(stderr, "Error: could not find function '%s'\n", next);
                    return unknown_bytecode;
                }
                if (!framesExpanded && vm->framesSoftMax != vm->framesHardMax && vm->fp + 1 >= vm->framesSoftMax - 2 && vm->fp + 1 < vm->framesHardMax - 1) {
                    if (verbose)
                        printf("INFO: Expanding number of frames from %hd to %hd\n", vm->framesSoftMax, vm->framesHardMax);
                    framesExpanded = true;
                    vm->callStack = realloc(vm->callStack, sizeof(Frame) * vm->framesHardMax);
                }
                if (vm->fp + 1 > vm->framesHardMax - 1) {
                    fprintf(stderr, "StackOverflow: Number of frames exceeded frame hard maximum of %hd\n", vm->framesHardMax);
                    return  memory_err;
                }
                Frame* frame = loadFrame(vm->src->code[addr].body, vm->src->code[addr].jumpPoints, vm->src->code[addr].jmpCnt, vm->stackSoftMax, vm->localsSoftMax, currentFrame->pc, argc, params);
                vm->callStack[++vm->fp] = frame;
            }
            
        }
        else if (strcmp(opcode, "RET") == 0) {
            rval = pop(vm);
            addr = currentFrame->returnAddr;
            Frame* caller = vm->callStack[--vm->fp];
            setPC(caller, addr);
            if (rval.type != None) {
                if (rval.type == Addr && rval.value.address != vm->globals) {
                    arrayTarget = checkAndRetrieveArrayValuesTarget(vm, caller, rval.size, &globalsExpanded, verbose);
                    if (vm->state != success)
                        return vm->state;
                    caller = arrayTarget.frame;
                    rval = copyAddr(rval, arrayTarget.targetp, &arrayTarget.target);
                }
                push(vm, rval, verbose);
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
            arrayTarget = checkAndRetrieveArrayValuesTarget(vm, currentFrame, capacity, &globalsExpanded, verbose);
            if (vm->state != success)
                return vm->state;
            currentFrame = arrayTarget.frame;
            rval = createAddr(arrayTarget.target, (*arrayTarget.targetp) + 1, capacity, argc);
            for (int i = 0; i < argc; i++) {
                arrayTarget.target[++(*arrayTarget.targetp)] = pop(vm);
            }
            if (capacity > argc) {
                for (int i = argc; i < capacity; i++) {
                    arrayTarget.target[++(*arrayTarget.targetp)] = createNone();
                }
            }
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "COPYARR") == 0) {
            rhs = pop(vm);
            arrayTarget = checkAndRetrieveArrayValuesTarget(vm, currentFrame, rhs.size, &globalsExpanded, verbose);
            if (vm->state != success)
                return vm->state;
            currentFrame = arrayTarget.frame;
            rval = copyAddr(rhs, arrayTarget.targetp, &arrayTarget.target);
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "AGET") == 0) {
            offset = pop(vm).value.intVal;
            lhs = pop(vm);
            DataConstant* start = getArrayStart(lhs);
            if (offset > lhs.size || offset < 0) {
                fprintf(stderr, "Error: Array index %d out of range %d\n", offset, lhs.size);
                return memory_err;
            }
            rval = *(start + offset);
            push(vm, rval, verbose);
        }
        else if (strcmp(opcode, "ASTORE") == 0) {
            offset = pop(vm).value.intVal;
            lhs = pop(vm);
            DataConstant* start = getArrayStart(lhs);
            if (offset >= lhs.size || offset < 0) {
                fprintf(stderr, "Error: Array index %d out of range %d\n", offset, lhs.size);
                return memory_err;
            }
            rhs = pop(vm);
            rval = *(start + offset);
            if (rval.type == None) {
                if (offset > lhs.length + 1) {
                    fprintf(stderr, "Error: Cannot write to index %d since previous index values are not initialized\n", offset);
                    return memory_err;
                }
                lhs.length++;
            }
            *(start + offset) = rhs;
            push(vm, lhs, verbose);
        }
        else {
            fprintf(stderr, "Unknown bytecode: '%s'\n", opcode);
            return unknown_bytecode;
        }
    }
    return success;
}