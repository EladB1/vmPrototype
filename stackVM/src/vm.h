#ifndef VM_H
#define VM_H

#include "filereader.h"
#include "frame.h"
#include "exitcode.h"
#include "config.h"

typedef struct {
    DataConstant* globals;
    SourceCode* src;
    Frame** callStack;
    int fp;
    int gp;
    ExitCode state;
    long frameSoftMax;
    long frameHardMax;
    long globalsSoftMax;
    long globalsHardMax;
} VM;

VM* init(SourceCode* src, KeyValue* config);
ExitCode run(VM* vm, bool verbose);
void destroy(VM* vm);

#endif 