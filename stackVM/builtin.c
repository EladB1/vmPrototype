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
        "_length_s",
        "_length_a",
        "max",
        "min",
        "_slice_s",
        "_slice_a",
        "_contains_s",
        "_contains_a",
        "toString",
        "_toInt_s",
        "_toInt_d",
        "_toDouble_s",
        "_toDouble_i",
        "at",
        "_reverse_s",
        "sleep",
        "exit"
    };
    int end = 18;
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
    if (strcmp(name, "_length_s") == 0)
        return createInt((int) strlen(params[0].value.strVal));
    if (strcmp(name, "_length_a") == 0)
        return createInt(params[0].size);
    if (strcmp(name, "max") == 0)
        return getMax(params[0], params[1]);
    if (strcmp(name, "min") == 0)
        return getMax(params[0], params[1]);
    if (strcmp(name, "_slice_s") == 0) {
        char* string = params[0].value.strVal;
        int end = argc == 2 ? (int) strlen(string) : params[2].value.intVal;
        return createString(slice(string, params[1].value.intVal, end));
    }
    if (strcmp(name, "_contains_s") == 0)
        return createBoolean(contains(params[0].value.strVal, params[1].value.strVal));
    if (strcmp(name, "toString") == 0)
        return createString(toString(params[0]));
    if (strcmp(name, "_toInt_s") == 0)
        return createInt(atoi(params[0].value.strVal));
    if (strcmp(name, "_toInt_d") == 0)
        return createInt((int) lround(params[0].value.dblVal));
    if (strcmp(name, "_toDouble_s") == 0)
        return createDouble(atof(params[0].value.strVal));
    if (strcmp(name, "_toDouble_i") == 0)
        return createDouble((double) params[0].value.intVal);
    if (strcmp(name, "at") == 0)
        return createString(at(params[0].value.strVal, params[1].value.intVal));
    if (strcmp(name, "_reverse_s") == 0)
        return createString(reverse(params[0].value.strVal));
    if (strcmp(name, "sleep") == 0)
        sleep_(params[0]);
    if (strcmp(name, "exit") == 0) {
        if (argc == 1)
            exit(params[0].value.intVal);
        exit(0);
    }
    return createNone();
}