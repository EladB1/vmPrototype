#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "dataconstant.h"

char* toString(DataConstant data) {
    char* string = "";
    if (data.type == Int)
        asprintf(&string, "%d", data.value.intVal);
    if (data.type == Dbl)
        asprintf(&string, "%f", data.value.dblVal);
    if (data.type == Addr)
        asprintf(&string, "0x%x (%d)", data.value.intVal, data.size);
    if (data.type == Bool)
        return data.value.boolVal ? "true" : "false";
    if (data.type == Str) {
        asprintf(&string, "\"%s\"", data.value.strVal);
    }
    if (data.type == Null)
        return "null";
    if (data.type == None)
        return "None";
    return string;
}

bool isZero(DataConstant data) {
    if (data.type == Dbl)
        return data.value.dblVal == 0;
    if (data.type == Int)
        return data.value.intVal == 0;
    return false;
}

bool isEqual(DataConstant lhs, DataConstant rhs) {
    if (lhs.type == Int)
        return rhs.type == Int ? lhs.value.intVal == rhs.value.intVal : lhs.value.intVal == rhs.value.dblVal;
    if (lhs.type == Dbl)
        return rhs.type == Dbl ? lhs.value.dblVal == rhs.value.dblVal : lhs.value.dblVal == rhs.value.intVal;
    if (lhs.type == Bool && rhs.type == Bool)
        return lhs.value.boolVal == rhs.value.boolVal;
    if (lhs.type == Str && rhs.type == Str) {
        if (strlen(lhs.value.strVal) != strlen(rhs.value.strVal))
            return false;
        return strcmp(lhs.value.strVal, rhs.value.strVal) == 0;
    }
    if (lhs.type == Null && rhs.type == Null)
        return true;
    return false;
}

/**
 * Cases:
 *  int (<, <=, >, >=) int
 *  double (<, <=, >, >=) double
 *  int (<, <=, >, >=) double
 *  double (<, <=, >, >=) int
 *  equality (any matching types)
 *  equality (any type with null)
 *  equality (int and double (either way))
*/
DataConstant compareData(DataConstant lhs, DataConstant rhs, char* comparison) {
    DataConstant result;
    result.type = Bool;
    result.size = 1;
    result.length = 1;
    if (strcmp(comparison, "==") == 0)
        result.value.boolVal = isEqual(lhs, rhs);
    else if (strcmp(comparison, "!="))
        result.value.boolVal = !isEqual(lhs, rhs);
    else {
        if ((lhs.type != Int && lhs.type != Dbl) || (rhs.type != Dbl && rhs.type != Int))
            result.value.boolVal = false;
        if (strcmp(comparison, "<") == 0) {
            if (lhs.type == Int)
                result.value.boolVal = rhs.type == Int ? lhs.value.intVal < rhs.value.intVal : lhs.value.intVal < rhs.value.dblVal;
            if (lhs.type == Dbl)
                result.value.boolVal = rhs.type == Dbl ? lhs.value.dblVal < rhs.value.dblVal : lhs.value.dblVal < rhs.value.intVal;
        }
        if (strcmp(comparison, "<=") == 0) {
            if (lhs.type == Int)
                result.value.boolVal = rhs.type == Int ? lhs.value.intVal <= rhs.value.intVal : lhs.value.intVal <= rhs.value.dblVal;
            if (lhs.type == Dbl)
                result.value.boolVal = rhs.type == Dbl ? lhs.value.dblVal <= rhs.value.dblVal : lhs.value.dblVal <= rhs.value.intVal;
        }
        if (strcmp(comparison, ">") == 0) {
            if (lhs.type == Int)
                result.value.boolVal = rhs.type == Int ? lhs.value.intVal > rhs.value.intVal : lhs.value.intVal > rhs.value.dblVal;
            if (lhs.type == Dbl)
                result.value.boolVal = rhs.type == Dbl ? lhs.value.dblVal > rhs.value.dblVal : lhs.value.dblVal > rhs.value.intVal;
        }
        if (strcmp(comparison, ">=") == 0) {
            if (lhs.type == Int)
                result.value.boolVal = rhs.type == Int ? lhs.value.intVal >= rhs.value.intVal : lhs.value.intVal >= rhs.value.dblVal;
            if (lhs.type == Dbl)
                result.value.boolVal = rhs.type == Dbl ? lhs.value.dblVal >= rhs.value.dblVal : lhs.value.dblVal >= rhs.value.intVal;
        }
    }
    return result;
}

DataConstant getMax(DataConstant lhs, DataConstant rhs) {
    return compareData(lhs, rhs, ">=").value.boolVal ? lhs : rhs;
}

DataConstant getMin(DataConstant lhs, DataConstant rhs) {
    return compareData(lhs, rhs, "<=").value.boolVal ? lhs : rhs;
}

DataConstant binaryArithmeticOperation(DataConstant lhs, DataConstant rhs, char* operation) {
    DataConstant result;
    if (strcmp(operation, "+") == 0) {
        if (lhs.type == Dbl || rhs.type == Dbl) {
            result.type = Dbl;
            if (lhs.type == rhs.type)
                result.value.dblVal = lhs.value.dblVal + rhs.value.dblVal;
            else if (lhs.type == Int)
                result.value.dblVal = lhs.value.intVal + rhs.value.dblVal;
            else
                result.value.dblVal = lhs.value.dblVal + rhs.value.intVal;
        }
        else {
            result.type = Int;
            result.value.intVal = lhs.value.intVal + rhs.value.intVal;
        }
    }
    else if (strcmp(operation, "-") == 0) {
        if (lhs.type == Dbl || rhs.type == Dbl) {
            if (lhs.type == rhs.type)
                result.value.dblVal = lhs.value.dblVal - rhs.value.dblVal;
            else if (lhs.type == Int)
                result.value.dblVal = lhs.value.intVal - rhs.value.dblVal;
            else
                result.value.dblVal = lhs.value.dblVal - rhs.value.intVal;
        }
        else {
            result.type = Int;
            result.value.intVal = lhs.value.intVal - rhs.value.intVal;
        }
    }
    if (strcmp(operation, "*") == 0) {
        if (lhs.type == Dbl || rhs.type == Dbl) {
            result.type = Dbl;
            if (lhs.type == rhs.type)
                result.value.dblVal = lhs.value.dblVal * rhs.value.dblVal;
            else if (lhs.type == Int)
                result.value.dblVal = lhs.value.intVal * rhs.value.dblVal;
            else
                result.value.dblVal = lhs.value.dblVal * rhs.value.intVal;
        }
        else {
            result.type = Int;
            result.value.intVal = lhs.value.intVal * rhs.value.intVal;
        }
    }
    if (strcmp(operation, "/") == 0) {
        if (isZero(rhs)) {
            fprintf(stderr, "Error: Division by zero\n");
            exit(1);
        }
        if (lhs.type == Dbl || rhs.type == Dbl) {
            result.type = Dbl;
            if (lhs.type == rhs.type)
                result.value.dblVal = lhs.value.dblVal / rhs.value.dblVal;
            else if (lhs.type == Int)
                result.value.dblVal = lhs.value.intVal / rhs.value.dblVal;
            else
                result.value.dblVal = lhs.value.dblVal / rhs.value.intVal;
        }
        else {
            result.type = Int;
            result.value.intVal = lhs.value.intVal / rhs.value.intVal;
        }
    }
    if (strcmp(operation, "mod") == 0) {
        if (isZero(rhs)) {
            fprintf(stderr, "Error: Division by zero\n");
            exit(1);
        }
        if (lhs.type == Dbl || rhs.type == Dbl) {
            result.type = Dbl;
            if (lhs.type == rhs.type)
                result.value.dblVal = fmod(lhs.value.dblVal, rhs.value.dblVal);
            else if (lhs.type == Int)
                result.value.dblVal = fmod(lhs.value.intVal, rhs.value.dblVal);
            else
                result.value.dblVal = fmod(lhs.value.dblVal, rhs.value.intVal);
        }
        else {
            result.type = Int;
            result.value.intVal = lhs.value.intVal % rhs.value.intVal;
        }
    }
    if (strcmp(operation, "exp") == 0) {
        if (lhs.type == Dbl || rhs.type == Dbl) {
            result.type = Dbl;
            if (lhs.type == rhs.type)
                result.value.dblVal = pow(lhs.value.dblVal, rhs.value.dblVal);
            else if (lhs.type == Int)
                result.value.dblVal = pow(lhs.value.intVal, rhs.value.dblVal);
            else
                result.value.dblVal = pow(lhs.value.dblVal, rhs.value.intVal);
        }
        else {
            result.type = Int;
            result.value.intVal = (int) lround(pow(lhs.value.intVal, rhs.value.intVal));
        }
    }
    result.size = 1;
    result.length = 1;
    return result;
}

DataConstant readInt(char* value) {
    DataConstant data;
    data.size = 1;
    data.length = 1;
    data.type = Int;
    data.value.intVal = atoi(value);
    return data;
}

DataConstant createInt(int value) {
    DataConstant data;
    data.size = 1;
    data.length = 1;
    data.type = Int;
    data.value.intVal = value;
    return data;
}

DataConstant readDouble(char* value) {
    DataConstant data;
    data.size = 1;
    data.length = 1;
    data.type = Dbl;
    data.value.dblVal = atof(value);
    return data;
}

DataConstant createDouble(double value) {
    DataConstant data;
    data.size = 1;
    data.length = 1;
    data.type = Dbl;
    data.value.dblVal = value;
    return data;
}

DataConstant readBoolean(char* value) {
    DataConstant data;
    data.type = Bool;
    data.size = 1;
    data.length = 1;
    data.value.boolVal = strcmp(value, "true") == 0;
    return data;
}

DataConstant createBoolean(bool value) {
    DataConstant data;
    data.type = Bool;
    data.size = 1;
    data.length = 1;
    data.value.boolVal = value;
    return data;
}

DataConstant createString(char* value) {
    DataConstant data;
    data.type = Str;
    data.size = 1;
    data.length = 1;
    data.value.strVal = value;
    return data;
}

DataConstant createNull() {
    DataConstant data;
    data.type = Null;
    data.size = 1;
    data.length = 1;
    return data;
}

DataConstant createNone() {
    DataConstant data;
    data.type = None;
    data.size = 0;
    data.length = 0;
    return data;
}

DataConstant createAddr(int addr, int capacity, int length) {
    DataConstant data;
    data.type = Addr;
    data.size = capacity;
    data.length = length;
    data.value.intVal = addr;
    return data;
}

DataConstant copyAddr(DataConstant src, int* addr, DataConstant** globals) {
    DataConstant copy;
    copy.type = Addr;
    copy.size = src.size;
    copy.length = src.length;
    copy.value.intVal = *addr + 1;
    int srcAddr = src.value.intVal;
    DataConstant* globs = *globals;
    for (int i = 0; i < src.size; i++) {
        globs[++(*addr)] = globs[srcAddr + i];
    }
    return copy;
}

DataConstant partialCopyAddr(DataConstant src, int start, int len, int* addr, DataConstant** globals) {
    DataConstant copy;
    copy.type = Addr;
    copy.size = src.size;
    copy.length = len;
    copy.value.intVal = *addr + 1;
    int srcAddr = start;
    DataConstant* globs = *globals;
    for (int i = 0; i < len && i < src.size; i++) {
        globs[++(*addr)] = globs[srcAddr + i];
    }
    if (len < src.size) {
        for (int i = len; i < src.size; i++) {
            globs[++(*addr)] = createNone();
        }
    }
    return copy;
}

DataConstant expandExistingAddr(DataConstant src, int capacity, int* addr, DataConstant** globals) {
    // TODO: remove old array from globals
    DataConstant copy;
    copy.type = Addr;
    copy.size = capacity;
    copy.length = src.length;
    copy.value.intVal = *addr + 1;
    int srcAddr = src.value.intVal;
    DataConstant* globs = *globals;
    for (int i = 0; i < src.size; i++) {
        globs[++(*addr)] = globs[srcAddr + i];
    }
    if (src.size < copy.size) {
        for (int i = src.size; i < copy.size; i++) {
            globs[++(*addr)] = createNone();
        }
    }
    return copy;
}