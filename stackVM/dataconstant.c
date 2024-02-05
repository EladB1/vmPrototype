#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "dataconstant.h"

char* toString(DataConstant data) {
    char string[64];
    if (data.type == Int)
        snprintf(string, 32, "%d", data.value.intVal);
    if (data.type == Dbl)
        snprintf(string, 64, "%f", data.value.dblVal);
    if (data.type == Bool)
        return data.value.boolVal ? "true" : "false";
    if (data.type == Str)
        return data.value.strVal;
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
    if (!(lhs.type == rhs.type && lhs.size == rhs.size))
        return false;
    if (lhs.type == Int)
        return lhs.value.intVal == rhs.value.intVal;
    if (lhs.type == Dbl)
        return lhs.value.dblVal == rhs.value.dblVal;
    if (lhs.type == Bool)
        return lhs.value.boolVal == rhs.value.boolVal;
    if (lhs.type == Str) {
        if (strlen(lhs.value.strVal) != strlen(rhs.value.strVal))
            return false;
        return strncmp(lhs.value.strVal, rhs.value.strVal, strlen(lhs.value.strVal));
    }
    return true;
}

DataConstant binaryArithmeticOperation(DataConstant lhs, DataConstant rhs, char* operation) {
    DataConstant result;
    if (strncmp(operation, "+", 2) == 0) {
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
    else if (strncmp(operation, "-", 2) == 0) {
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
    if (strncmp(operation, "*", 2) == 0) {
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
    if (strncmp(operation, "/", 2) == 0) {
        if (isZero(rhs)) {
            printf("Error: Division by zero\n");
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
    if (strncmp(operation, "mod", 4) == 0) {
        if (isZero(rhs)) {
            printf("Error: Division by zero\n");
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
    result.size = 1;
    return result;
}

DataConstant toAddress(int value) {
    DataConstant member;
    member.type = Addr;
    member.size = 1;
    member.value.intVal = value;
    return member;
}
