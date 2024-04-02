#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/vm.h"

TestSuite(VM);

Test(VM, initAndDestroy) {
    char* labels[2] = {"add", "_entry"};
    char* bodies[2] = {
        "LOAD 0 LOAD 1 ADD RET",
        "LOAD_CONST 1 LOAD_CONST 5 LE JMPT .end LOAD_CONST 3 CALL println 1 .end: HALT"
    };
    int jumpCounts[2] = {0, 1};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}, (JumpPoint[]) {{".end", 14}}};
    SourceCode src = createSource(labels, bodies, jumpCounts, jumps, 2);
    
    VM* vm = init(src);

    cr_expect_eq(vm->fp, 0);
    cr_expect_eq(vm->gc, -1);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->returnAddr, 0);
    cr_expect_eq(frame->pc, 0);
    cr_expect_eq(frame->sp, -1);
    cr_expect_eq(frame->jc, 1);
    cr_expect_eq(frame->lc, -1);

    destroy(vm);
    deleteSource(src);
}

Test(VM, runNoEntryPoint, .init = cr_redirect_stderr, .exit_code = 255) {
    char* labels[1] = {"add"};
    char* bodies[1] = {
        "HALT"
    };
    int jumpCounts[1] = {0};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}};
    SourceCode src = createSource(labels, bodies, jumpCounts, jumps, 1);

    VM* vm = init(src);
    
    cr_expect_stderr_eq_str("Error: Could not find entry point function label: '_entry'");

    destroy(vm);
    deleteSource(src);
}

Test(VM, runWithSimpleJump) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 1 LOAD_CONST 5 LE JMPT .end LOAD_CONST 3 LOAD_CONST 4 LOAD_CONST 5 HALT .end: HALT"
    };
    int jumpCounts[1] = {1};
    JumpPoint* jumps[1] = {(JumpPoint[]) {{".end", 15}}};
    SourceCode src = createSource(labels, bodies, jumpCounts, jumps, 1);

    //displayCode(src);

    VM* vm = init(src);
    run(vm, false);

    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 16);
    //cr_log_info("PC: %d\n", frame->pc);
    cr_expect_eq(frame->pc, 16);
    cr_expect_eq(frame->sp, -1);

    destroy(vm);
    deleteSource(src);
}

Test(VM, runLoadBasicConsts) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 1 LOAD_CONST 5.5 LOAD_CONST false  LOAD_CONST \"HI\" LOAD_CONST NULL LOAD_CONST NONE HALT "
    }; // tests doesn't read last token without extra space at the end since split() is called differently between the app and tests
    // TODO: Fix bug in comment above
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode src = createSource(labels, bodies, jumpCounts, jumps, 1);
    //displayCode(src);

    VM* vm = init(src);
    run(vm, false);

    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 13);
    //cr_log_info("PC: %d\n", frame->pc);
    cr_expect_eq(frame->pc, 13);
    cr_expect_eq(frame->sp, 5);

    cr_expect(isEqual(frame->stack[0], createInt(1)));
    cr_expect(isEqual(frame->stack[1], createDouble(5.5)));
    cr_expect(isEqual(frame->stack[2], createBoolean(false)));
    cr_expect(isEqual(frame->stack[3], createString("HI")));
    cr_expect(isEqual(frame->stack[4], createNull()));
    cr_expect_eq(frame->stack[5].type, None);

    destroy(vm);
    deleteSource(src);
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
    char* bodies[1] = {
        ""
    };
    cr_asprintf(&bodies[0], "LOAD_CONST 1 DUP %s HALT ", comparisons->operator); 
    // tests doesn't read last token without extra space at the end since split() is called differently between the app and tests
    // TODO: Fix bug in comment above
    int jumpCounts[1] = {0};
    JumpPoint* jumps[1] = {(JumpPoint[]) {}};
    SourceCode src = createSource(labels, bodies, jumpCounts, jumps, 1);
    //displayCode(src);

    VM* vm = init(src);
    run(vm, false);

    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 5);
    //cr_log_info("PC: %d\n", frame->pc);
    cr_expect_eq(frame->pc, 5);
    cr_expect_eq(frame->sp, 0);

    cr_expect_eq(frame->stack[0].type, Bool);
    cr_expect_eq(frame->stack[0].value.boolVal, comparisons->result);

    destroy(vm);
    deleteSource(src);
}

Test(VM, runWithNoJump) {
    char* labels[1] = {"_entry"};
    char* bodies[1] = {
        "LOAD_CONST 1 LOAD_CONST 5 GT JMPT .end LOAD_CONST 3 LOAD_CONST 4 LOAD_CONST 5 HALT .end: HALT"
    };
    int jumpCounts[1] = {1};
    JumpPoint* jumps[1] = {(JumpPoint[]) {{".end", 18}}};
    SourceCode src = createSource(labels, bodies, jumpCounts, jumps, 1);

    //displayCode(src);

    VM* vm = init(src);
    run(vm, false);
    
    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 16);
    //cr_log_info("PC: %d\n", frame->pc);
    cr_expect_eq(frame->pc, 14);
    cr_expect_eq(frame->sp, 2);
    
    destroy(vm);
    deleteSource(src);
}

Test(VM, runWithFunctionCall_no_params_stop) {
    char* labels[2] = {"end", "_entry"};
    char* bodies[2] = {
        "HALT",
        "CALL end 0 HALT"
    };
    int jumpCounts[2] = {0, 0};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}, (JumpPoint[]) {}};
    SourceCode src = createSource(labels, bodies, jumpCounts, jumps, 2);

    //displayCode(src);

    VM* vm = init(src);
    run(vm, false);

    cr_expect_eq(vm->fp, 1);
    Frame* frame = vm->callStack[1];
    cr_expect_eq(frame->instructions->length, 1);
    //cr_log_info("PC: %d\n", frame->pc);
    cr_expect_eq(frame->pc, 1);
    cr_expect_eq(frame->sp, -1);

    destroy(vm);
    deleteSource(src);
}

Test(VM, runWithFunctionCall_withParams_stop) {
    char* labels[2] = {"end", "_entry"};
    char* bodies[2] = {
        "LOAD 0 HALT",
        "LOAD_CONST 1 LOAD_CONST true CALL end 2 HALT"
    };
    int jumpCounts[2] = {0, 0};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}, (JumpPoint[]) {}};
    SourceCode src = createSource(labels, bodies, jumpCounts, jumps, 2);

    //displayCode(src);

    VM* vm = init(src);
    run(vm, false);

    cr_expect_eq(vm->fp, 1);
    Frame* frame = vm->callStack[1];
    cr_expect_eq(frame->instructions->length, 3);
    //cr_log_info("PC: %d\n", frame->pc);
    cr_expect_eq(frame->pc, 3);
    cr_expect_eq(frame->sp, 0);
    cr_expect_eq(frame->lc, 1);
    cr_expect(isEqual(frame->locals[0], frame->stack[0]));
    cr_expect(isEqual(frame->locals[1], createInt(1)));

    destroy(vm);
    deleteSource(src);
}

Test(VM, runWithFunctionCall_addWithReturn) {
    char* labels[2] = {"add", "_entry"};
    char* bodies[2] = {
        "LOAD 0 LOAD 1 ADD RET",
        "LOAD_CONST 1 LOAD_CONST 2.718 CALL add 2 HALT"
    };
    int jumpCounts[2] = {0, 0};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}, (JumpPoint[]) {}};
    SourceCode src = createSource(labels, bodies, jumpCounts, jumps, 2);

    //displayCode(src);

    VM* vm = init(src);
    run(vm, false);

    cr_expect_eq(vm->fp, 0);
    Frame* frame = vm->callStack[0];
    cr_expect_eq(frame->instructions->length, 8);
    //cr_log_info("PC: %d\n", frame->pc);
    cr_expect_eq(frame->pc, 8);
    cr_expect_eq(frame->sp, 0);
    cr_expect_eq(frame->lc, -1);
    cr_expect_eq(frame->stack[0].type, Dbl);
    cr_expect_eq(frame->stack[0].value.dblVal, 3.718);

    destroy(vm);
    deleteSource(src);
}