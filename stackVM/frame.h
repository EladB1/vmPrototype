#include "dataconstant.h"
#include "stringvector.h"
#define STACK_SIZE 100

typedef struct {
    DataConstant* stack;
    DataConstant* locals;
    StringVector* instructions;
    int pc;
    int sp;
    int lc;
    int frameAddr;
    int returnAddr;
} Frame;

Frame* loadFrame(StringVector*, int, int, int, DataConstant*);
void deleteFrame(Frame*);
void framePush(Frame*, DataConstant);
DataConstant framePop(Frame*);
DataConstant frameTop(Frame*);
char* getNextInstruction(Frame*);
char* peekNextInstruction(Frame*);
DataConstant loadLocal(Frame*, int);
void storeLocal(Frame*, DataConstant);
void storeLocalAtAddr(Frame*, DataConstant, int);
void incrementPC(Frame*);
void setPC(Frame*, int);
void print_array(char*, DataConstant*, int);
bool stackIsEmpty(Frame*);