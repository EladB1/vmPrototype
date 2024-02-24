#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "frame.h"

#define STACK_SIZE 256

Frame* loadFrame(StringVector* code, JumpPoint* jumps, int jc, int pc, int argc, DataConstant* params) {
    Frame* frame = malloc(sizeof(Frame));
    frame->instructions = code;
    frame->returnAddr = pc;
    frame->jumps = jumps;
    frame->jc = jc;
    frame->pc = 0;
    frame->lc = -1;
    frame->sp = -1;
    frame->stack = malloc(sizeof(DataConstant) * STACK_SIZE);
    frame->locals = malloc(sizeof(DataConstant) * STACK_SIZE);
    for (int i = 0; i < argc; i++) {
        frame->locals[++frame->lc] = params[i];
    }
    return frame;
}

void deleteFrame(Frame* frame) {
    free(frame->locals);
    free(frame->stack);
    free(frame);
}

void framePush(Frame* frame, DataConstant value) {
    frame->stack[++frame->sp] = value;
}

DataConstant framePop(Frame* frame) {
    return frame->stack[frame->sp--];
}

DataConstant frameTop(Frame* frame) {
    return frame->stack[frame->sp];
}

char* getNextInstruction(Frame* frame) {
    return getFromSV(frame->instructions, frame->pc++);
}

char* peekNextInstruction(Frame* frame) {
    return getFromSV(frame->instructions, frame->pc);
}

DataConstant loadLocal(Frame* frame, int addr) {
    return frame->locals[addr];
}

void storeLocal(Frame* frame, DataConstant value) {
    frame->locals[++frame->lc] = value;
}

void storeLocalAtAddr(Frame* frame, DataConstant value, int addr) { // overwrite existing variable
    frame->locals[addr] = value;
}

void incrementPC(Frame* frame) {
    frame->pc++;
}

void setPC(Frame* frame, int addr) {
    frame->pc = addr;
}

int getJumpIndex(Frame* frame, char* label) {
    JumpPoint jmp;
    for (int i = 0; i < frame->jc; i++) {
        jmp = frame->jumps[i];
        if (strcmp(jmp.label, label) == 0)
            return jmp.index;
    }
    return -1;
}

void print_array(char* array_label, DataConstant* array, int array_size) {
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

bool stackIsEmpty(Frame* frame) {
    return frame->sp == -1;
}