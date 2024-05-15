#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdbool.h>

#include "vm.h"
#include "frame.h"
#include "dataconstant.h"
#include "exitcode.h"

bool isBuiltinFunction(char* name);
DataConstant callBuiltinFunction(char* name, int argc, DataConstant* params, VM* vm, Frame* frame, bool* globalsExpanded, bool verbose);

#endif