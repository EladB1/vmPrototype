#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtin.h"
#include "impl_builtin.h"

bool isBuiltinFunction(char* name) {
    return strcmp(name, "sleep") == 0 || strcmp(name, "at") == 0;
}

DataConstant callBuiltin(char* name, int argc, DataConstant* params) {
    if (strcmp(name, "sleep") == 0)
        return sleep_(params[0]);
    if (strcmp(name, "at") == 0)
        return at(params[0].value.strVal, params[1].value.intVal);
    return createNone();
}