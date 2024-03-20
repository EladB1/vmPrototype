#ifndef BUILTIN_H
#define BUILTIN_H

#include "dataconstant.h"

bool isBuiltinFunction(char* name);
DataConstant callBuiltinFunction(char* name, int argc, DataConstant* params, int* globCount, DataConstant** globals);

#endif