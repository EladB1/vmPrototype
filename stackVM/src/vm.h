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
    bool useHeapStorageBackup;
    short framesSoftMax;
    short framesHardMax;
    long globalsSoftMax;
    long globalsHardMax;
    long localsSoftMax;
    long localsHardMax;
    long stackSoftMax;
    long stackHardMax;
} VM;

typedef struct {
    DataConstant* target;
    int* targetp;
    Frame* frame;
} ArrayTarget;

VM* init(SourceCode* src, VMConfig conf);
ExitCode run(VM* vm, bool verbose);
void destroy(VM* vm);

#endif 