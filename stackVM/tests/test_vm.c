#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <criterion/redirect.h>

#include "utils.h"
#include "../src/vm.h"

TestSuite(VM);

Test(VM, initAndDestroy) {
    SourceCode src;
    src.length = 2;
    StringVector* addBody = split("LOAD 0 LOAD 1 ADD RET", " ");
    StringVector* mainBody = split("LOAD_CONST 1 LOAD_CONST 5 LE JMPT .end LOAD_CONST 3 CALL println 1 .end: HALT", " ");
    src.code[0].label = "add";
    src.code[0].body = addBody;
    src.code[0].jmpCnt = 0;
    src.code[1].label = "_entry";
    src.code[1].body = mainBody;
    src.code[1].jumpPoints[0] = (JumpPoint) {".end", 14};
    src.code[1].jmpCnt = 1;

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
    free(addBody);
    free(mainBody);
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
    char* labels[2] = {"add", "_entry"};
    char* bodies[2] = {
        "LOAD 0 LOAD 1 ADD RET",
        "LOAD_CONST 1 LOAD_CONST 5 LE JMPT .end LOAD_CONST 3 LOAD_CONST 4 LOAD_CONST 5 HALT .end: HALT"
    };
    int jumpCounts[2] = {0, 1};
    JumpPoint* jumps[2] = {(JumpPoint[]) {}, (JumpPoint[]) {{".end", 15}}};
    SourceCode src = createSource(labels, bodies, jumpCounts, jumps, 2);

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