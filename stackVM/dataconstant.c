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
    if (lhs.type == Int)
        return rhs.type == Int ? lhs.value.intVal == rhs.value.intVal : lhs.value.intVal == rhs.value.dblVal;
    if (lhs.type == Dbl)
        return rhs.type == Dbl ? lhs.value.dblVal == rhs.value.dblVal : lhs.value.dblVal == rhs.value.intVal;
    if (lhs.type == Bool && rhs.type == Bool)
        return lhs.value.boolVal == rhs.value.boolVal;
    if (lhs.type == Str && rhs.type == Str) {
        if (strlen(lhs.value.strVal) != strlen(rhs.value.strVal))
            return false;
        return strcmp(lhs.value.strVal, rhs.value.strVal);
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
