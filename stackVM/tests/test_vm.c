#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/vm.h"

void testRuntimeError(char* body, char* errorMessage, ExitCode vmState, bool verbose) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = { body };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    if (verbose)
        displayCode(src);

    VM* vm = init(src, getDefaultConfig());
    ExitCode status = run(vm, verbose);
    
    cr_expect_stderr_eq_str(errorMessage);
    cr_expect_eq(status, vmState);

    destroy(vm);
    cr_free(src);
}

TestSuite(VM);

Test(VM, initAndDestroy) {
    char* labels[2] = {"add", "_entry"};
    char* bodies[2] = {
        "LOAD 0 LOAD 1 ADD RET",
        "LOAD_CONST 1 LOAD_CONST 5 LE JMPT .end LOAD_CONST 3 CALL println 1 .end: HALT EJMP"
    };
    int jumpCounts[2] = {0, 1};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}, (JumpPoint[]) {{".end", 14, 15}}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 2);
    
    VM* vm = init(src, getDefaultConfig());

    cr_expect_not_null(vm);
    cr_expect_eq(vm->fp, 0);
    cr_expect_eq(vm->gp, -1);
    cr_expect_eq(vm->state, success);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->returnAddr, 0);
    cr_expect_eq(frame->pc, 0);
    cr_expect_eq(frame->sp, -1);
    cr_expect_eq(frame->jc, 1);
    cr_expect_eq(frame->lp, -1);

    destroy(vm);
    cr_free(src);
    
}

Test(VM, noEntryPoint, .init = cr_redirect_stderr) {
    char* labels[1] = {"add"};
    char* bodies[1] = {
        "HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    
    cr_expect_null(vm);
    cr_expect_stderr_eq_str("Error: Could not find entry point function label: '_entry'\n");
}

Test(VM, runUknownBytecode, .init = cr_redirect_stderr) {
    testRuntimeError("asdf HALT", "Unknown bytecode: 'asdf'\n", unknown_bytecode, false);
}

Test(VM, runJumpPointNotFound, .init = cr_redirect_stderr) {
    testRuntimeError( "JMP .next HALT", "Error: Could not find jump point '.next'\n", unknown_bytecode, false);
}

Test(VM, runPop) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST true POP HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 4);
    cr_expect_eq(frame->pc, 4);
    cr_expect_eq(frame->sp, -1);

    destroy(vm);
    cr_free(src);
}



Test(VM, runLoadBasicConsts) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 1 LOAD_CONST 5.5 LOAD_CONST false  LOAD_CONST \"HI\" LOAD_CONST NULL LOAD_CONST NONE HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 13);
    cr_expect_eq(frame->pc, 13);
    cr_expect_eq(frame->sp, 5);

    cr_expect(isEqual(frame->stack[0], createInt(1)));
    cr_expect(isEqual(frame->stack[1], createDouble(5.5)));
    cr_expect(isEqual(frame->stack[2], createBoolean(false)));
    cr_expect(isEqual(frame->stack[3], createString("HI")));
    cr_expect(isEqual(frame->stack[4], createNull()));
    cr_expect_eq(frame->stack[5].type, None);

    destroy(vm);
    cr_free(src);
}

Test(VM, runStringOps) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST \"HI\" REPEATSTR 3 LOAD_CONST \"IH\" REPEATSTR 3 CONCAT HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 10);
    cr_expect_eq(frame->pc, 10);
    cr_expect_eq(frame->sp, 0);

    cr_expect(isEqual(frame->stack[0], createString("HIHIHIIHIHIH")));
    
    destroy(vm);
    cr_free(src);
}

typedef struct {
    char* operator;
    bool result;
} DataCompare;

ParameterizedTestParameters(VM, runDUPAndCompare) {
    size_t count = 6;
    DataCompare* values = cr_malloc(sizeof(DataCompare) * count);
    values[0] = (DataCompare) {cr_strdup("EQ"), true};
    values[1] = (DataCompare) {cr_strdup("NE"), false};
    values[2] = (DataCompare) {cr_strdup("LT"), false};
    values[3] = (DataCompare) {cr_strdup("LE"), true};
    values[4] = (DataCompare) {cr_strdup("GT"), false};
    values[5] = (DataCompare) {cr_strdup("GE"), true};
    return cr_make_param_array(DataCompare, values, count);
}

ParameterizedTest(DataCompare* comparisons, VM, runDUPAndCompare) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {""};
    cr_asprintf(&bodies[0], "LOAD_CONST 1 DUP %s HALT", comparisons->operator); 
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 5);
    cr_expect_eq(frame->pc, 5);
    cr_expect_eq(frame->sp, 0);

    cr_expect_eq(frame->stack[0].type, Bool);
    cr_expect_eq(frame->stack[0].value.boolVal, comparisons->result);

    destroy(vm);
    cr_free(src);
}

Test(VM, runNOT) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST true NOT LOAD_CONST false NOT HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 7);
    cr_expect_eq(frame->pc, 7);
    cr_expect_eq(frame->sp, 1);

    cr_expect(isEqual(frame->stack[0], createBoolean(false)));
    cr_expect(isEqual(frame->stack[1], createBoolean(true)));

    destroy(vm);
    cr_free(src);

}

typedef struct {
    char* lhs;
    char* rhs;
    char* operator;
    bool result;
} BooleanOperation;

ParameterizedTestParameters(VM, runBinaryBooleanOperations) {
    size_t count = 8;
    BooleanOperation* values = cr_malloc(sizeof(BooleanOperation) * count);
    values[0] = (BooleanOperation) {cr_strdup("true"), cr_strdup("true"), cr_strdup("AND"), true};
    values[1] = (BooleanOperation) {cr_strdup("false"), cr_strdup("true"), cr_strdup("AND"), false};
    values[2] = (BooleanOperation) {cr_strdup("true"), cr_strdup("false"), cr_strdup("AND"), false};
    values[3] = (BooleanOperation) {cr_strdup("false"), cr_strdup("false"), cr_strdup("AND"), false};
    values[4] = (BooleanOperation) {cr_strdup("true"), cr_strdup("true"), cr_strdup("OR"), true};
    values[5] = (BooleanOperation) {cr_strdup("false"), cr_strdup("true"), cr_strdup("OR"), true};
    values[6] = (BooleanOperation) {cr_strdup("true"), cr_strdup("false"), cr_strdup("OR"), true};
    values[7] = (BooleanOperation) {cr_strdup("false"), cr_strdup("false"), cr_strdup("OR"), false};
    return cr_make_param_array(BooleanOperation, values, count);
}

ParameterizedTest(BooleanOperation* operation, VM, runBinaryBooleanOperations) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        ""
    };
    cr_asprintf(&bodies[0], "LOAD_CONST %s LOAD_CONST %s %s HALT", operation->lhs, operation->rhs, operation->operator); 
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 6);
    cr_expect_eq(frame->pc, 6);
    cr_expect_eq(frame->sp, 0);

    cr_expect_eq(frame->stack[0].type, Bool);
    cr_expect_eq(frame->stack[0].value.boolVal, operation->result);

    destroy(vm);
    cr_free(src);
}

typedef struct {
    char* lhs;
    char* rhs;
    char* operator;
    int result;
} BitwiseOperation;

ParameterizedTestParameters(VM, runBinaryBitwiseOperations) {
    size_t count = 13;
    BitwiseOperation* values = cr_malloc(sizeof(BitwiseOperation) * count);
    values[0] = (BitwiseOperation) {cr_strdup("true"), cr_strdup("true"), cr_strdup("XOR"), 0};
    values[1] = (BitwiseOperation) {cr_strdup("false"), cr_strdup("true"), cr_strdup("XOR"), 1};
    values[2] = (BitwiseOperation) {cr_strdup("1"), cr_strdup("false"), cr_strdup("XOR"), 1};
    values[3] = (BitwiseOperation) {cr_strdup("false"), cr_strdup("0"), cr_strdup("XOR"), 0};
    values[4] = (BitwiseOperation) {cr_strdup("12"), cr_strdup("12"), cr_strdup("XOR"), 0};
    values[5] = (BitwiseOperation) {cr_strdup("12"), cr_strdup("11"), cr_strdup("XOR"), 7};
    values[6] = (BitwiseOperation) {cr_strdup("true"), cr_strdup("true"), cr_strdup("B_AND"), 1};
    values[7] = (BitwiseOperation) {cr_strdup("false"), cr_strdup("true"), cr_strdup("B_AND"), 0};
    values[8] = (BitwiseOperation) {cr_strdup("1"), cr_strdup("false"), cr_strdup("B_AND"), 0};
    values[9] = (BitwiseOperation) {cr_strdup("false"), cr_strdup("0"), cr_strdup("B_AND"), 0};
    values[10] = (BitwiseOperation) {cr_strdup("12"), cr_strdup("12"), cr_strdup("B_AND"), 12};
    values[11] = (BitwiseOperation) {cr_strdup("12"), cr_strdup("11"), cr_strdup("B_AND"), 8};
    values[12] = (BitwiseOperation) {cr_strdup("12"), cr_strdup("false"), cr_strdup("B_AND"), 0};
    return cr_make_param_array(BitwiseOperation, values, count);
}

ParameterizedTest(BitwiseOperation* operation, VM, runBinaryBitwiseOperations) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {""};
    cr_asprintf(&bodies[0], "LOAD_CONST %s LOAD_CONST %s %s HALT", operation->lhs, operation->rhs, operation->operator); 
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 6);
    cr_expect_eq(frame->pc, 6);
    cr_expect_eq(frame->sp, 0);

    cr_expect_eq(frame->stack[0].type, Int);
    cr_expect_eq(frame->stack[0].value.intVal, operation->result);

    destroy(vm);
    cr_free(src);
}

Test(VM, runLoadAndStore) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST true STORE LOAD 0 NOT STORE 0 LOAD_CONST 3 GSTORE GLOAD 0 LOAD_CONST 1 SUB GSTORE 0 LOAD 0 HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 21);
    cr_expect_eq(frame->pc, 21);
    cr_expect_eq(frame->sp, 0);
    cr_expect_eq(frame->lp, 0);
    cr_expect_eq(vm->gp, 0);

    cr_expect(isEqual(frame->stack[0], createBoolean(false)));
    cr_expect(isEqual(frame->locals[0], createBoolean(false)));
    cr_expect(isEqual(vm->globals[0], createInt(2)));

    destroy(vm);
    cr_free(src);
}

Test(VM, runWithNoJump) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 1 LOAD_CONST 5 GT JMPT .end LOAD_CONST 3 LOAD_CONST 4 LOAD_CONST 5 HALT .end: HALT EJMP"
    };
    int jumpCounts[1] = {1};
    JumpPoint* jumps[1] = {(JumpPoint[]) {{".end", 18, 19}}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);
    
    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 17);
    cr_expect_eq(frame->pc, 14);
    cr_expect_eq(frame->sp, 2);
    
    destroy(vm);
    cr_free(src);
}

Test(VM, runWithSimpleJump) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 1 LOAD_CONST 5 LE JMPT .end LOAD_CONST 3 LOAD_CONST 4 LOAD_CONST 5 HALT .end: HALT EJMP"
    };
    int jumpCounts[1] = {1};
    JumpPoint* jumps[1] = {(JumpPoint[]) {{".end", 15, 16}}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 17);
    cr_expect_eq(frame->pc, 16);
    cr_expect_eq(frame->sp, -1);

    destroy(vm);
    cr_free(src);
}

Test(VM, runWithShortCircuitJump) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 1 LOAD_CONST 5 GT SJMPF .end LOAD_CONST 3 LOAD_CONST 4 LOAD_CONST 5 HALT .end: HALT EJMP"
    };
    int jumpCounts[1] = {1};
    JumpPoint* jumps[1] = {(JumpPoint[]) {{".end", 15, 16}}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 17);
    cr_expect_eq(frame->pc, 16);
    cr_expect_eq(frame->sp, 0);

    cr_expect(isEqual(frame->stack[0], createBoolean(false)));

    destroy(vm);
    cr_free(src);
}

Test(VM, runSelect) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST true SELECT 1 2 LOAD_CONST false SELECT 3 4 HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 11);
    cr_expect_eq(frame->pc, 11);
    cr_expect_eq(frame->sp, 1);

    cr_expect(isEqual(frame->stack[0], createInt(1)));
    cr_expect(isEqual(frame->stack[1], createInt(4)));

    destroy(vm);
    cr_free(src);
}

Test(VM, runWithBuiltinFunctionCall, .init = cr_redirect_stdout) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST \"Hello, world!\" CALL println 1 HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);
    fflush(stdout);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 6);
    cr_expect_eq(frame->pc, 6);
    cr_expect_eq(frame->sp, -1);

    cr_expect_stdout_eq_str("Hello, world!\n");

    destroy(vm);
    cr_free(src);
}

Test(VM, runWithFunctionCall_no_params_stop) {
    char* labels[2] = {"end", "_entry"};
    char* bodies[2] = {
        "HALT",
        "CALL end 0 HALT"
    };
    int jumpCounts[2] = {0, 0};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}, (JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 2);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 1);
    Frame* frame = vm->callStack[1];
    cr_expect_eq(frame->instructions->length, 1);
    cr_expect_eq(frame->pc, 1);
    cr_expect_eq(frame->sp, -1);

    destroy(vm);
    cr_free(src);
}

Test(VM, runWithFunctionCall_withParams_stop) {
    char* labels[2] = {"end", "_entry"};
    char* bodies[2] = {
        "LOAD 0 HALT",
        "LOAD_CONST 1 LOAD_CONST true CALL end 2 HALT"
    };
    int jumpCounts[2] = {0, 0};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}, (JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 2);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 1);
    Frame* frame = vm->callStack[1];
    cr_expect_eq(frame->instructions->length, 3);
    cr_expect_eq(frame->pc, 3);
    cr_expect_eq(frame->sp, 0);
    cr_expect_eq(frame->lp, 1);
    cr_expect(isEqual(frame->locals[0], frame->stack[0]));
    cr_expect(isEqual(frame->locals[1], createInt(1)));

    destroy(vm);
    cr_free(src);
}

Test(VM, runWithFunctionCall_addWithReturn) {
    char* labels[2] = {"add", "_entry"};
    char* bodies[2] = {
        "LOAD 0 LOAD 1 ADD RET",
        "LOAD_CONST 1 LOAD_CONST 2.718 CALL add 2 HALT"
    };
    int jumpCounts[2] = {0, 0};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}, (JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 2);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 8);
    cr_expect_eq(frame->pc, 8);
    cr_expect_eq(frame->sp, 0);
    cr_expect_eq(frame->lp, -1);
    cr_expect_eq(frame->stack[0].type, Dbl);
    cr_expect_eq(frame->stack[0].value.dblVal, 3.718);

    destroy(vm);
    cr_free(src);
}

Test(VM, runWithFunctionCall_arrayReturn) {
    char* labels[2] = {"test", "_entry"};
    char* bodies[2] = {
        "LOAD_CONST 2 LOAD_CONST 1 BUILDARR 2 2 RET",
        "LOAD_CONST true STORE CALL test 0 HALT"
    };
    int jumpCounts[2] = {0, 0};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}, (JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 2);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 7);
    cr_expect_eq(frame->pc, 7);
    cr_expect_eq(frame->sp, 0);
    cr_expect_eq(frame->lp, 2);

    cr_expect_eq(frame->stack[0].type, Addr);
    cr_expect_eq(frame->stack[0].value.address, frame->locals);
    cr_expect_eq(frame->stack[0].offset, 1);
    cr_expect_eq(frame->stack[0].size, 2);
    cr_expect_eq(frame->stack[0].length, 2);
    
    cr_expect_eq(frame->locals[1].value.intVal, 1);
    cr_expect_eq(frame->locals[2].value.intVal, 2);

    destroy(vm);
    cr_free(src);
}

Test(VM, runWithFunctionCall_nestedArrayReturn) {
    char* labels[2] = {"test", "_entry"};
    char* bodies[2] = {
        "LOAD_CONST 2 LOAD_CONST 1 BUILDARR 2 2 BUILDARR 2 1 RET",
        "LOAD_CONST true STORE CALL test 0 HALT"
    };
    int jumpCounts[2] = {0, 0};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}, (JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 2);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 7);
    cr_expect_eq(frame->pc, 7);
    cr_expect_eq(frame->sp, 0);
    cr_expect_eq(frame->lp, 4);

    cr_expect_eq(frame->stack[0].type, Addr);
    cr_expect_eq(frame->stack[0].value.address, frame->locals);
    cr_expect_eq(frame->stack[0].offset, 3);
    cr_expect_eq(frame->stack[0].size, 2);
    cr_expect_eq(frame->stack[0].length, 1);

    cr_expect_eq(frame->locals[1].value.intVal, 1);
    cr_expect_eq(frame->locals[2].value.intVal, 2);
    cr_expect_eq(frame->locals[3].type, Addr);
    cr_expect_eq(frame->locals[3].value.address, frame->locals);
    cr_expect_eq(frame->locals[3].offset, 1);
    cr_expect_eq(frame->locals[3].size, 2);
    cr_expect_eq(frame->locals[3].length, 2);
    cr_expect_eq(frame->locals[4].type, None);

    destroy(vm);
    cr_free(src);
}

Test(VM, runUknownFunction, .init = cr_redirect_stderr) {
    testRuntimeError("CALL asdf 0 HALT", "Error: could not find function 'asdf'\n", unknown_bytecode, false);
}

Test(VM, runArrayGet) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 2 LOAD_CONST 1 BUILDARR 5 2 LOAD_CONST 1 AGET HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 11);
    cr_expect_eq(frame->pc, 11);
    cr_expect_eq(frame->sp, 0);
    cr_expect_eq(vm->gp, -1);
    cr_expect_eq(frame->lp, 4);

    cr_expect(isEqual(frame->stack[0], createInt(2)));
    cr_expect(isEqual(frame->locals[0], createInt(1)));
    cr_expect(isEqual(frame->locals[1], createInt(2)));
    for (int i = 2; i < 5; i++) {
        cr_expect_eq(frame->locals[i].type, None);
    }

    destroy(vm);
    cr_free(src);
}

Test(VM, buildArrayOneParam) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 2 BUILDARR 0 HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 5);
    cr_expect_eq(frame->pc, 5);
    cr_expect_eq(frame->sp, 1);
    cr_expect_eq(vm->gp, -1);
    cr_expect_eq(frame->lp, 1);

    cr_expect(isEqual(frame->stack[0], createInt(2)));
    cr_expect_eq(frame->stack[1].type, Addr);
    cr_expect_eq(frame->stack[1].length, 0);
    cr_expect_eq(frame->stack[1].size, 2);
    cr_expect_eq(frame->stack[1].offset, 0);
    cr_expect_eq(frame->locals[0].type, None);
    cr_expect_eq(frame->locals[1].type, None);

    destroy(vm);
    cr_free(src);
}

Test(VM, runArrayWrite) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 2 LOAD_CONST 1 BUILDARR 5 2 STORE LOAD_CONST 0 LOAD 5 LOAD_CONST 1 ASTORE STORE 5 LOAD_CONST 4 LOAD 5 LOAD_CONST 2 ASTORE STORE 5 HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 27);
 
    cr_expect_eq(frame->pc, 27);
    cr_expect_eq(frame->sp, -1);
    cr_expect_eq(vm->gp, -1);
    cr_expect_eq(frame->lp, 5);

    cr_expect_eq(frame->locals[5].type, Addr);
    cr_expect_eq(frame->locals[5].value.address, frame->locals);
    cr_expect_eq(frame->locals[5].offset, 0);
    cr_expect_eq(frame->locals[5].length, 3);
    cr_expect_eq(frame->locals[5].size, 5);
    cr_expect(isEqual(frame->locals[0], createInt(1)));
    cr_expect(isEqual(frame->locals[1], createInt(0)));
    cr_expect(isEqual(frame->locals[2], createInt(4)));
    for (int i = 3; i < 5; i++) {
        cr_expect_eq(frame->locals[i].type, None);
    }

    destroy(vm);
    cr_free(src);
}

Test(VM, runArrayConcat) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 2 LOAD_CONST 1 BUILDARR 3 2 LOAD_CONST 1 LOAD_CONST 0 BUILDARR 2 2 CONCAT HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 16);
 
    cr_expect_eq(frame->pc, 16);
    cr_expect_eq(frame->sp, 0);
    cr_expect_eq(vm->gp, -1);
    cr_expect_eq(frame->lp, 9);

    cr_expect_eq(frame->stack[0].type, Addr);
    cr_expect_eq(frame->stack[0].length, 4);
    cr_expect_eq(frame->stack[0].size, 5);

    cr_expect(isEqual(frame->locals[0], createInt(1)));
    cr_expect(isEqual(frame->locals[1], createInt(2)));
    cr_expect_eq(frame->locals[2].type, None);
    cr_expect(isEqual(frame->locals[3], createInt(0)));
    cr_expect(isEqual(frame->locals[4], createInt(1)));
    cr_expect(isEqual(frame->locals[5], createInt(1)));
    cr_expect(isEqual(frame->locals[6], createInt(2)));
    cr_expect(isEqual(frame->locals[7], createInt(0)));
    cr_expect(isEqual(frame->locals[8], createInt(1)));
    cr_expect_eq(frame->locals[9].type, None);

    destroy(vm);
    cr_free(src);
}

Test(VM, runArrayCopy) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 2 LOAD_CONST 1 BUILDARR 2 2 DUP COPYARR HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 10);
 
    cr_expect_eq(frame->pc, 10);
    cr_expect_eq(frame->sp, 1);
    cr_expect_eq(vm->gp, -1);
    cr_expect_eq(frame->lp, 3);

    cr_expect_eq(frame->stack[0].type, Addr);
    cr_expect_eq(frame->stack[0].offset, 0);
    cr_expect_eq(frame->stack[0].length, 2);
    cr_expect_eq(frame->stack[0].size, 2);
    cr_expect_eq(frame->stack[1].type, Addr);
    cr_expect_eq(frame->stack[1].offset, 2);
    cr_expect_eq(frame->stack[1].length, 2);
    cr_expect_eq(frame->stack[1].size, 2);
    cr_expect(isEqual(frame->locals[0], createInt(1)));
    cr_expect(isEqual(frame->locals[1], createInt(2)));
    cr_expect(isEqual(frame->locals[2], createInt(1)));
    cr_expect(isEqual(frame->locals[3], createInt(2)));

    destroy(vm);
    cr_free(src);
}

Test(VM, runArrayGlobalStore) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 3.14 GSTORE LOAD_CONST 2 LOAD_CONST 1 BUILDARR 5 2 GSTORE HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 12);
    cr_expect_eq(frame->pc, 12);
    cr_expect_eq(frame->sp, -1);
    cr_expect_eq(vm->gp, 6);
    cr_expect_eq(frame->lp, 4);

    cr_expect(isEqual(frame->locals[0], createInt(1)));
    cr_expect(isEqual(frame->locals[1], createInt(2)));
    for (int i = 2; i < 5; i++) {
        cr_expect_eq(frame->locals[i].type, None);
    }

    cr_expect(isEqual(vm->globals[1], createInt(1)));
    cr_expect(isEqual(vm->globals[2], createInt(2)));
    for (int i = 3; i < 6; i++) {
        cr_expect_eq(vm->globals[i].type, None);
    }

    cr_expect_eq(vm->globals[6].type, Addr);
    cr_expect_eq(vm->globals[6].value.address, vm->globals);
    cr_expect_eq(vm->globals[6].offset, 1);
    cr_expect_eq(vm->globals[6].length, 2);
    cr_expect_eq(vm->globals[6].size, 5);

    destroy(vm);
    cr_free(src);
}

Test(VM, runArrayGlobalStoreNested) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 3.14 GSTORE LOAD_CONST 2 LOAD_CONST 1 BUILDARR 2 2 BUILDARR 2 0 LOAD_CONST 1 BUILDARR 2 1 BUILDARR 4 3 GSTORE HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode* src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src, getDefaultConfig());
    bool verbose = false;
    if (verbose)
        displayCode(src);
    ExitCode status = run(vm, verbose);

    cr_expect_eq(status, success);
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 23);
    cr_expect_eq(frame->pc, 23);
    cr_expect_eq(frame->sp, -1);
    cr_expect_eq(vm->gp, 11);
    cr_expect_eq(frame->lp, 9);

    // locals
    cr_expect(isEqual(frame->locals[0], createInt(1)));
    cr_expect(isEqual(frame->locals[1], createInt(2)));
    cr_expect_eq(frame->locals[2].type, None);
    cr_expect_eq(frame->locals[3].type, None);
    cr_expect(isEqual(frame->locals[4], createInt(1)));
    cr_expect_eq(frame->locals[5].type, None);

    cr_expect_eq(frame->locals[6].type, Addr);
    // NOTE: GSTORE will change the local address to a global one for each nested array, but we don't need those values anymore so it doesn't matter
    // cr_expect_eq(frame->locals[6].value.address, vm->globals);
    cr_expect_eq(frame->locals[6].offset, 1);
    cr_expect_eq(frame->locals[6].size, 2);
    cr_expect_eq(frame->locals[6].length, 1);
    cr_expect_eq(frame->locals[7].type, Addr);
    cr_expect_eq(frame->locals[7].offset, 3);
    cr_expect_eq(frame->locals[7].size, 2);
    cr_expect_eq(frame->locals[7].length, 0);
    cr_expect_eq(frame->locals[8].type, Addr);
    cr_expect_eq(frame->locals[8].offset, 5);
    cr_expect_eq(frame->locals[8].size, 2);
    cr_expect_eq(frame->locals[8].length, 2);
    cr_expect_eq(frame->locals[9].type, None);

    // globals
    cr_expect(isEqual(vm->globals[1], createInt(1)));
    cr_expect_eq(vm->globals[2].type, None);
    cr_expect_eq(vm->globals[3].type, None);
    cr_expect_eq(vm->globals[4].type, None);
    cr_expect(isEqual(vm->globals[5], createInt(1)));
    cr_expect(isEqual(vm->globals[6], createInt(2)));

    cr_expect_eq(vm->globals[7].type, Addr);
    cr_expect_eq(vm->globals[7].value.address, vm->globals);
    cr_expect_eq(vm->globals[7].offset, 1);
    cr_expect_eq(vm->globals[7].size, 2);
    cr_expect_eq(vm->globals[7].length, 1);
    cr_expect_eq(vm->globals[8].type, Addr);
    cr_expect_eq(vm->globals[8].value.address, vm->globals);
    cr_expect_eq(vm->globals[8].offset, 3);
    cr_expect_eq(vm->globals[8].size, 2);
    cr_expect_eq(vm->globals[8].length, 0);
    cr_expect_eq(vm->globals[9].type, Addr);
    cr_expect_eq(vm->globals[9].value.address, vm->globals);
    cr_expect_eq(vm->globals[9].offset, 5);
    cr_expect_eq(vm->globals[9].size, 2);
    cr_expect_eq(vm->globals[9].length, 2);
    cr_expect_eq(vm->globals[10].type, None);

    cr_expect_eq(vm->globals[11].type, Addr);
    cr_expect_eq(vm->globals[11].value.address, vm->globals);
    cr_expect_eq(vm->globals[11].offset, 7);
    cr_expect_eq(vm->globals[11].size, 4);
    cr_expect_eq(vm->globals[11].length, 3);

    destroy(vm);
    cr_free(src);
}

Test(VM, runArrayInvalidBuild, .init = cr_redirect_stderr) {
    testRuntimeError(
        "LOAD_CONST 1 LOAD_CONST 2 BUILDARR 1 2 HALT",
        "Error: Attempted to build array of length 2 which exceeds capacity 1\n",
        memory_err,
        false
    );
}

Test(VM, runArrayGetOutOfRange, .init = cr_redirect_stderr) {
    testRuntimeError(
        "LOAD_CONST 1 LOAD_CONST 2 BUILDARR 2 2 LOAD_CONST 10 AGET HALT",
        "Error: Array index 10 out of range 2\n",
        memory_err,
        false
    );
}

Test(VM, runArrayStoreOutOfRange, .init = cr_redirect_stderr) {
    testRuntimeError(
        "LOAD_CONST 1 LOAD_CONST 2 BUILDARR 2 2 LOAD_CONST 10 ASTORE HALT",
        "Error: Array index 10 out of range 2\n", 
        memory_err,
        false
    );
}

Test(VM, runArrayWritePastLength, .init = cr_redirect_stderr) {
    testRuntimeError(
        "LOAD_CONST 1 LOAD_CONST 2 BUILDARR 12 2 LOAD_CONST 10 ASTORE HALT",
        "Error: Cannot write to index 10 since previous index values are not initialized\n",
        memory_err,
        false
    );
}