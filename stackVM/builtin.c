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
        "printerr",
        "_length_s",
        "_length_a",
        "capacity",
        "getType",
        "max",
        "min",
        "_slice_s",
        "_slice_a",
        "_contains_s",
        "_contains_a",
        "indexOf",
        "toString",
        "_toInt_s",
        "_toInt_d",
        "_toDouble_s",
        "_toDouble_i",
        "at",
        "_join_1",
        "_join_2",
        "_reverse_s",
        "_reverse_a",
        "sort",
        "sleep",
        "exit",
        "fileExists",
        "createFile",
        "writeToFile",
        "appendToFile",
        "renameFile",
        "deleteFile"
    };
    int end = 32;
    for (int i = 0; i < end; i++) {
        if (strcmp(name, builtins[i]) == 0)
            return true;
    }
    return false;
}

DataConstant callBuiltin(char* name, int argc, DataConstant* params, DataConstant** globals) {
    if (strcmp(name, "print") == 0)
        print(params[0], *globals, false);
    if (strcmp(name, "println") == 0)
        print(params[0], *globals, true);
    if (strcmp(name, "printerr") == 0) {
        if (argc == 1)
            printerr(params[0], false, 0);
        else
            printerr(params[0], params[1].value.boolVal, params[2].value.intVal);
    }
    if (strcmp(name, "_length_s") == 0)
        return createInt((int) strlen(params[0].value.strVal));
    if (strcmp(name, "_length_a") == 0)
        return createInt(params[0].length);
    if (strcmp(name, "capacity") == 0)
        return createInt(params[0].size);
    if (strcmp(name, "getType") == 0)
        return createString(getType(params[0], *globals));
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
    if (strcmp(name, "_contains_a") == 0)
        return createBoolean(arrayContains(params[0], params[1], *globals));
    if (strcmp(name, "indexOf") == 0)
        return createInt(indexOf(params[0], params[1], *globals));
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
    if (strcmp(name, "_join_1") == 0)
        return createString(join(params[0], "", *globals));
    if (strcmp(name, "_join_2") == 0)
        return createString(join(params[0], params[1].value.strVal, *globals));
    if (strcmp(name, "_reverse_s") == 0)
        return createString(reverse(params[0].value.strVal));
    if (strcmp(name, "sort") == 0)
        sort(params[0], *globals);
    if (strcmp(name, "sleep") == 0)
        sleep_(params[0]);
    if (strcmp(name, "exit") == 0) {
        if (argc == 1)
            exit(params[0].value.intVal);
        exit(0);
    }
    if (strcmp(name, "fileExists") == 0)
        return createBoolean(fileExists(params[0].value.strVal));
    if (strcmp(name, "createFile") == 0)
        createFile(params[0].value.strVal);
    if (strcmp(name, "writeToFile") == 0)
        writeToFile(params[0].value.strVal, params[1].value.strVal, "w");
    if (strcmp(name, "appendToFile") == 0)
        writeToFile(params[0].value.strVal, params[1].value.strVal, "a");
    if (strcmp(name, "renameFile") == 0)
        renameFile(params[0].value.strVal, params[1].value.strVal);
    if (strcmp(name, "deleteFile") == 0)
        deleteFile(params[0].value.strVal);
    return createNone();
}