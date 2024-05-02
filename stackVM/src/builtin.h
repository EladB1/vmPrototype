#ifndef BUILTIN_H
#define BUILTIN_H

#include "dataconstant.h"
#include "exitcode.h"

bool isBuiltinFunction(char* name);
DataConstant callBuiltinFunction(char* name, int argc, DataConstant* params, int* lp, DataConstant** locals, ExitCode* vmState);

#endif