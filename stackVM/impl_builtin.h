#ifndef IMPL_BUILTIN_H
#define IMPL_BUILTIN_H

#include "dataconstant.h"

DataConstant sleep_(DataConstant seconds);
DataConstant at(char* str, int index);
DataConstant startsWith(char* string, char* prefix);

#endif