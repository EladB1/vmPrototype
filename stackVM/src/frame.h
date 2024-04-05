#ifndef FRAME_H
#define FRAME_H

#include "dataconstant.h"
#include "stringvector.h"
#include "filereader.h"

typedef struct {
    DataConstant* stack;
    DataConstant* locals;
    StringVector* instructions;
    JumpPoint* jumps;
    int jc;
    int pc;
    int sp;
    int lp;
    int returnAddr;
} Frame;

Frame* loadFrame(StringVector* code, JumpPoint* jumps, int jc, int pc, int argc, DataConstant* params);
void deleteFrame(Frame* frame);
void framePush(Frame* frame, DataConstant value);
DataConstant framePop(Frame* frame);
DataConstant frameTop(Frame* frame);
char* getNextInstruction(Frame* frame);
char* peekNextInstruction(Frame* frame);
DataConstant loadLocal(Frame* frame, int addr);
void storeLocal(Frame* frame, DataConstant value);
void storeLocalAtAddr(Frame*, DataConstant value, int addr);
void incrementPC(Frame* frame);
void setPC(Frame* frame, int addr);
int getJumpIndex(Frame* frame, char* label);
void print_array(char* array_label, DataConstant* array, int array_size);
bool stackIsEmpty(Frame* frame);

#endif