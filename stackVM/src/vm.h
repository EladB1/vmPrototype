#ifndef VM_H
#define VM_H

#include "filereader.h"
#include "frame.h"
#include "exitcode.h"

typedef struct {
    DataConstant* globals;
    SourceCode* src;
    Frame** callStack;
    int fp;
    int gp;
    ExitCode state;
} VM;

VM* init(SourceCode* src);
ExitCode run(VM* vm, bool verbose);
void destroy(VM* vm);

#endif 