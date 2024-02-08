#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "builtin.h"
#include "impl_builtin.h"

bool isBuiltinFunction(char* name) {
    char* builtins[] = {
        "print",
        "println",
        "_toInt_s",
        "_toInt_d",
        "at",
        "reverse",
        "sleep"
    };
    int end = 7;
    for (int i = 0; i < end; i++) {
        if (strcmp(name, builtins[i]) == 0)
            return true;
    }
    return false;
}

DataConstant callBuiltin(char* name, int argc, DataConstant* params) {
    if (strcmp(name, "print") == 0)
        print(params[0], false);
    if (strcmp(name, "println") == 0)
        print(params[0], true);
    if (strcmp(name, "toString") == 0)
        return createString(toString(params[0]));
    if (strcmp(name, "_toInt_s") == 0)
        return createInt(atoi(params[0].value.strVal));
    if (strcmp(name, "_toInt_d") == 0)
        return createInt((int) lround(params[0].value.dblVal));
    if (strcmp(name, "at") == 0)
        return createString(at(params[0].value.strVal, params[1].value.intVal));
    if (strcmp(name, "reverse") == 0)
        return createString(reverse(params[0].value.strVal));
    if (strcmp(name, "sleep") == 0)
        sleep_(params[0]);
    
    return createNone();
}