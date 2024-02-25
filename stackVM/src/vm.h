#ifndef VM_H
#define VM_H

#include "filereader.h"
#include "frame.h"

typedef struct {
    DataConstant* globals;
    SourceCode src;
    Frame** callStack;
    int fp;
    int gc;
} VM;

VM* init(SourceCode src);
void run(VM* vm, bool verbose);
void destroy(VM* vm);

#endif 