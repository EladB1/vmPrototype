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
        "replace",
        "replaceAll",
        "_slice_s",
        "_slice_a",
        "append",
        "prepend",
        "insert",
        "_remove_indx_a",
        "_remove_val_a",
        "_remove_all_val_a",
        "_contains_s",
        "_contains_a",
        "indexOf",
        "toString",
        "_toInt_s",
        "_toInt_d",
        "_toDouble_s",
        "_toDouble_i",
        "at",
        "join",
        "_reverse_s",
        "_reverse_a",
        "sort",
        "sleep",
        "exit",
        "fileExists",
        "createFile",
        "readFile",
        "writeToFile",
        "appendToFile",
        "renameFile",
        "deleteFile",
        "getEnv",
        "setEnv"
    };
    int end = 42;
    for (int i = 0; i < end; i++) {
        if (strcmp(name, builtins[i]) == 0)
            return true;
    }
    return false;
}

DataConstant callBuiltinFunction(char* name, int argc, DataConstant* params, int* lp, DataConstant** locals, ExitCode* vmState) {
    if (strcmp(name, "print") == 0)
        print(params[0], false);
    if (strcmp(name, "println") == 0)
        print(params[0], true);
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
        return createString(getType(params[0]));
    if (strcmp(name, "max") == 0)
        return getMax(params[0], params[1]);
    if (strcmp(name, "min") == 0)
        return getMax(params[0], params[1]);
    if (strcmp(name, "replace") == 0)
        return createString(replace(params[0].value.strVal, params[1].value.strVal, params[2].value.strVal, false));
    if (strcmp(name, "replaceAll") == 0)
        return createString(replace(params[0].value.strVal, params[1].value.strVal, params[2].value.strVal, true));
    if (strcmp(name, "_slice_s") == 0) {
        char* string = params[0].value.strVal;
        int end = argc == 2 ? (int) strlen(string) : params[2].value.intVal;
        return createString(slice(string, params[1].value.intVal, end, vmState));
    }
    if (strcmp(name, "_slice_a") == 0) {
        DataConstant array = params[0];
        int end = argc == 2 ? array.length : params[2].value.intVal;
        return sliceArr(array, params[1].value.intVal, end, lp, locals, vmState);
    }
    if (strcmp(name, "append") == 0) {
        DataConstant array = params[0];
        append(&array, params[1], vmState);
        return array;
    }
    if (strcmp(name, "prepend") == 0) {
        DataConstant array = params[0];
        prepend(&array, params[1], vmState);
        return array;
    }
    if (strcmp(name, "insert") == 0) {
        DataConstant array = params[0];
        insert(&array, params[1], params[2].value.intVal, vmState);
        return array;
    }
    if (strcmp(name, "_remove_indx_a") == 0) {
        int index = params[1].value.intVal;
        removeByIndex(&params[0], index, vmState);
        return params[0];
    }
    if (strcmp(name, "_remove_val_a") == 0) {
        int index = indexOf(params[0], params[1]);
        if (index != -1)
            removeByIndex(&params[0], index, vmState);
        return params[0];
    }
    if (strcmp(name, "_remove_all_val_a") == 0) {
        int index = indexOf(params[0], params[1]);
        while (index != -1) {
            removeByIndex(&params[0], index, vmState);
            index = indexOf(params[0], params[1]);
        }
        return params[0];
    }
    if (strcmp(name, "_contains_s") == 0)
        return createBoolean(contains(params[0].value.strVal, params[1].value.strVal));
    if (strcmp(name, "_contains_a") == 0)
        return createBoolean(arrayContains(params[0], params[1]));
    if (strcmp(name, "indexOf") == 0)
        return createInt(indexOf(params[0], params[1]));
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
        return createString(at(params[0].value.strVal, params[1].value.intVal, vmState));
    if (strcmp(name, "join") == 0) {
        char* delim = argc == 1 ? "" : params[1].value.strVal;
        return createString(join(params[0], delim));
    }
    if (strcmp(name, "_reverse_s") == 0)
        return createString(reverse(params[0].value.strVal));
    if (strcmp(name, "_reverse_a") == 0) {
        reverseArr(params[0]);
        return params[0];
    }
    if (strcmp(name, "sort") == 0)
        sort(params[0]);
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
        createFile(params[0].value.strVal, vmState);
    if (strcmp(name, "readFile") == 0)
        return readFile(params[0].value.strVal, lp, locals, vmState);
    if (strcmp(name, "writeToFile") == 0)
        writeToFile(params[0].value.strVal, params[1].value.strVal, "w", vmState);
    if (strcmp(name, "appendToFile") == 0)
        writeToFile(params[0].value.strVal, params[1].value.strVal, "a", vmState);
    if (strcmp(name, "renameFile") == 0)
        renameFile(params[0].value.strVal, params[1].value.strVal, vmState);
    if (strcmp(name, "deleteFile") == 0)
        deleteFile(params[0].value.strVal, vmState);
    if (strcmp(name, "getEnv") == 0)
        return createString(getenv(params[0].value.strVal));
    if (strcmp(name, "setEnv") == 0) {
        char* envStr;
        asprintf(&envStr, "%s=%s", params[0].value.strVal, params[1].value.strVal);
        int set = putenv(envStr);
        if (set != 0) {
            fprintf(stderr, "Failed to set environment variable\n");
            *vmState = (ExitCode) set;
        }
    }
    return createNone();
}