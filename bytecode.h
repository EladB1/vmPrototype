typedef enum {
    ADD_I32,
    SUB_I32,
    MUL_I32,
    LT_I32,
    EQ_I32,
    JMP, // branch
    JMPT, // branch if true
    JMPF, // branch if false
    CONST_I32,
    LOAD, // load from local
    GLOAD, // load from global
    STORE, // store to local
    GSTORE, // store to global
    PRINT,
    POP,
    DUP,
    HALT,
    CALL,
    RET
} opcode;