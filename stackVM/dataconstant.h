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

char* toString(DataConstant);
bool isZero(DataConstant);
bool isEqual(DataConstant, DataConstant);
DataConstant binaryArithmeticOperation(DataConstant, DataConstant, char*);
DataConstant toAddress(int);