typedef enum {
    Addr = 1,
    Int,
    Dbl,
    Str,
    Arr,
    Bool
} Type;

typedef union memberVal {
    int intVal;
    double dblVal;
    bool boolVal;
    char* strVal;
    union memberVal* arrayVal;
} MemberValue;

typedef struct {
    Type type;
    MemberValue value;
    int size;
} StackMember;

char* toString(StackMember);
bool isZero(StackMember);
bool isEqual(StackMember, StackMember);
StackMember binaryArithmeticOperation(StackMember, StackMember, char*);
