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
    bool expandedStack;
    bool expandedLocals;
} Frame;

Frame* loadFrame(StringVector* code, JumpPoint* jumps, int jc, long stackSize, long localsSize, int pc, int argc, DataConstant* params);
void deleteFrame(Frame* frame);
Frame* expandStack(Frame* frame, long stackSize);
Frame* expandLocals(Frame* frame, long localsSize);
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
int getJumpStart(Frame* frame, char* label);
int getJumpEnd(Frame* frame, char* label);
void print_array(char* array_label, DataConstant* array, int array_size);
bool stackIsEmpty(Frame* frame);

#endif