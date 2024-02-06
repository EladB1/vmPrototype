#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtin.h"
#include "impl_builtin.h"

bool isBuiltinFunction(char* name) {
    return strcmp(name, "reverse") == 0 || strcmp(name, "sleep") == 0 || strcmp(name, "at") == 0;
}

DataConstant callBuiltin(char* name, int argc, DataConstant* params) {
    if (strcmp(name, "sleep") == 0)
        sleep_(params[0]);
    if (strcmp(name, "at") == 0)
        return createString(at(params[0].value.strVal, params[1].value.intVal));
    if (strcmp(name, "reverse") == 0)
        return createString(reverse(params[0].value.strVal));
    return createNone();
}