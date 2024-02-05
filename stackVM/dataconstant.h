#ifndef DATACONSTANT_H
#define DATACONSTANT_H

#include <stdbool.h>

typedef enum {
    Addr = 1,
    Int,
    Dbl,
    Str,
    Arr,
    Bool,
    Null,
    None
} Datatype;

typedef union memberVal {
    int intVal;
    double dblVal;
    bool boolVal;
    char* strVal;
    union memberVal* arrayVal;
} DataValue;

typedef struct {
    Datatype type;
    DataValue value;
    int size;
} DataConstant;

char* toString(DataConstant data);
bool isZero(DataConstant data);
DataConstant compareData(DataConstant lhs, DataConstant rhs, char* comparison);
DataConstant binaryArithmeticOperation(DataConstant lhs, DataConstant rhs, char* operation);
DataConstant toAddress(int value);

#endif