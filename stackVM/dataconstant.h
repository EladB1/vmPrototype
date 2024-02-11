#ifndef DATACONSTANT_H
#define DATACONSTANT_H

#include <stdbool.h>

typedef enum {
    Addr = 1,
    Int,
    Dbl,
    Str,
    Bool,
    Null,
    None
} Datatype;

typedef union memberVal {
    int intVal;
    double dblVal;
    bool boolVal;
    char* strVal;
} DataValue;

typedef struct {
    Datatype type;
    DataValue value;
    int size;
    int length;
} DataConstant;

char* toString(DataConstant data);
bool isZero(DataConstant data);
DataConstant compareData(DataConstant lhs, DataConstant rhs, char* comparison);
DataConstant getMax(DataConstant lhs, DataConstant rhs);
DataConstant getMin(DataConstant lhs, DataConstant rhs);
DataConstant binaryArithmeticOperation(DataConstant lhs, DataConstant rhs, char* operation);
DataConstant readInt(char* value);
DataConstant createInt(int value);
DataConstant readDouble(char* value);
DataConstant createDouble(double value);
DataConstant readBoolean(char* value);
DataConstant createBoolean(bool value);
DataConstant createString(char* value);
DataConstant createNull();
DataConstant createNone();
DataConstant createAddr(int addr, int capacity, int length);

#endif